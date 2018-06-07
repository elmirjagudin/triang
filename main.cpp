#include <stdio.h>
#include <iostream>
#include <iomanip>

#define CERES_FOUND 1      // hack to make sure reconstruct() stuff is defined
#include <opencv2/sfm.hpp>
#include <opencv2/viz.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;
using namespace cv::sfm;

#define F   664.383695213819
#define CX  609.7365486001763
#define CY  600.5922443696466

enum
{
  B1 = 0,
  C1,
  C2,
  C3,
  C4,
  M1,
  M2,
  M3,
  M4,
  NUM_POINTS, // needs to be last
};

void
set_point(Mat_<double> & frame, int point_num, double x, double y)
{
//    cout << x << ", " << y << endl;
    frame(0, point_num) = x;
    frame(1, point_num) = y;
}

const char *
point_name(int num)
{
    switch (num)
    {
        case B1:
            return "B1";
        case C1:
            return "C1";
        case C2:
            return "C2";
        case C3:
            return "C3";
        case C4:
            return "C4";
        case M1:
            return "M1";
        case M2:
            return "M2";
        case M3:
            return "M3";
        case M4:
            return "M4";
        default:
            return "?";
    }
}

void
show_points(string fname, Mat_<double> & points)
{
    auto fpath =
        "/home/boris/Desktop/RectifiedImgs/CameraRig/fb0/png/" + fname + ".png";

    Mat image;
    image = imread(fpath, CV_LOAD_IMAGE_COLOR);

    if (!image.data)
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }


    cout << fpath << endl;

    for (int i = 0; i < NUM_POINTS; i += 1)
    {
        Point p1(points(0, i),
                 points(1, i));
        Point p2(points(0, i),
                 points(1, i)-50);

        line(image, p1, p2, Scalar( 110, 220, 0 ),  2, 8);
        putText(image, point_name(i), p2, 0, 1, Scalar( 110, 220, 0 ), 2);
    }

    Mat tmp;
//    Mat tmp = Mat(1200, 1200, CV_8UC3, pixels);
    resize(image, tmp, Size(), .8, .8);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", tmp );

    waitKey(0);

}

void
init_points(vector<Mat> &points2d)
{
    Mat_<double> frame(2, NUM_POINTS);

    /* img0.png */
    set_point(frame, B1, 471,  660);

    set_point(frame, C1, 54,   706);
    set_point(frame, C2, 298,  625);
    set_point(frame, C3, 744,  687);
    set_point(frame, C4, 786,  828);

    set_point(frame, M1, 304, 1019);
    set_point(frame, M2, 456,  956);
    set_point(frame, M3, 531, 1071);
    set_point(frame, M4, 351, 1178);
cout << frame << endl;

show_points("img0", frame);
    points2d.push_back(Mat(frame));

    /* img12.png */
    frame = Mat_<double>(2, NUM_POINTS);
    set_point(frame, B1, 457, 653);

    set_point(frame, C1, 22,  695);
    set_point(frame, C2, 276, 615);
    set_point(frame, C3, 729, 680);
    set_point(frame, C4, 771, 823);

    set_point(frame, M1, 277, 1014);
    set_point(frame, M2, 438,  950);
    set_point(frame, M3, 512, 1064);
    set_point(frame, M4, 329, 1171);
show_points("img12", frame);
    points2d.push_back(Mat(frame));

    /* img16.png */
    frame = Mat_<double>(2, NUM_POINTS);
    set_point(frame, B1, 585,  658);

    set_point(frame, C1, 220,  675);
    set_point(frame, C2, 420,  614);
    set_point(frame, C3, 867,  704);
    set_point(frame, C4, 916,  856);

    set_point(frame, M1, 411,  987);
    set_point(frame, M2, 555,  948);
    set_point(frame, M3, 624, 1074);
    set_point(frame, M4, 451, 1143);

show_points("img16", frame);
    points2d.push_back(Mat(frame));

    /* img20.png */
    frame = Mat_<double>(2, NUM_POINTS);
    set_point(frame, B1, 671,  666);

    set_point(frame, C1, 325,  674);
    set_point(frame, C2, 508,  620);
    set_point(frame, C3, 969,  721);
    set_point(frame, C4, 1029, 891);

    set_point(frame, M1, 494,  984);
    set_point(frame, M2, 636,  957);
    set_point(frame, M3, 705, 1094);
    set_point(frame, M4, 532, 1142);

show_points("img20", frame);
    points2d.push_back(Mat(frame));
}

void show_synth_points(Mat & points)
{
    Mat img(1000, 1000, CV_8UC3, Scalar(256, 256, 256));

    auto col = Scalar(0, 0, 0);
    for (int i = 0; i < points.rows; i += 1)
    {
        auto x = points.at<float>(i, 0);
        auto y = points.at<float>(i, 1);

        rectangle(img, Point(x-1, y-1), Point(x+1, y+1), col);
    }

    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", img );

    waitKey(0);
}

#define Z_DIST 20

static void
synth(float yrot, float t, Mat & imagePoints)
{
    vector<Point3f> points;

    points.push_back(Point3f(0, 0, Z_DIST-10));
    points.push_back(Point3f(0, 5, Z_DIST-5));
    points.push_back(Point3f(5, 0, Z_DIST-5));
    points.push_back(Point3f(0, -5, Z_DIST-5));
    points.push_back(Point3f(-5, 0, Z_DIST-5));

    for (int i = -5; i <= 5; i += 1)
    {
         points.push_back(Point3f((float)i, 5, Z_DIST));
         points.push_back(Point3f((float)i, -5, Z_DIST));
    }

    for (int i = -4; i <= 4; i += 1)
    {
         points.push_back(Point3f(5, (float)i, Z_DIST));
         points.push_back(Point3f(-5, (float)i, Z_DIST));
    }

    // for (int i = 0; i < points.size(); i += 1)
    // {
    //     auto p = points[i];
    //     cout << p.x << " " << p.y << " " << p.z << endl;
    // }

    // cout << "yrot " << yrot << " t " << t << endl;

    Mat rvec = (Mat_<float>(3,1) << 0, yrot, 0);
    Mat tvec = (Mat_<float>(3,1) << t, 0, 0);

    Matx33d K = Matx33d(500, 0, 500,
                        0, 500, 500,
                        0, 0,  1);

    Mat dist;

    projectPoints(points, rvec, tvec, K, noArray(), imagePoints);
//    show_synth_points(imagePoints);
}

void
init_synth_points(vector<Mat> & points2d)
{
    for (int i = -6;
         i < -4; //i <= 6;
         i += 1)
    {
        Mat imagePoints;

        auto yr = ((float)i) / 30;
        auto t = ((float)i) / 10;
        synth(yr, t, imagePoints);


        Mat_<double> frame(2, imagePoints.rows);
        for (int j = 0; j < imagePoints.rows; j += 1)
        {
//            cout << imagePoints.at<float>(j, 0) << " " <<
//                imagePoints.at<float>(j, 1) << endl;
            set_point(frame, j,
                      imagePoints.at<float>(j, 0),
                      imagePoints.at<float>(j, 1));
        }

        points2d.push_back(Mat(frame));

//cout << imagePoints << endl;
    }
}

const Matx33d K = Matx33d(500, 0, 500,
                          0, 500, 500,
                          0, 0,  1);

void
triangulate(vector<Mat> & points2d)
{
    Mat P1, P2;
    Mat R;
    Mat rvec;
    Mat t = (Mat_<double>(3,1) << -0.6, 0, 0);

    rvec = (Mat_<double>(3,1) << 0, -0.2, 0);
    Rodrigues(rvec, R);
    projectionFromKRt(K, R, t, P1);

    rvec = (Mat_<double>(3,1) << 0, -0.166667, 0);
    Rodrigues(rvec, R);
    t = (Mat_<double>(3,1) << -0.5, 0, 0);
    projectionFromKRt(K, R, t, P2);

    cout << "P1 " << P1 << endl;
    cout << "P2 " << P2 << endl;
    vector<Mat> Ps;
    Ps.push_back(P1);
    Ps.push_back(P2);

    // vector<Mat> points2d;

    // Mat x = (Mat_<double>(2,5) <<
    //     398.645, 398.645, 561.178, 398.645, 212.556,
    //     500,     670.056, 500,     329.944, 500);
    // cout << x << endl;
    // points2d.push_back(x);

    // x = (Mat_<double>(2,5) <<
    //     415.886, 415.886, 578.17,  415.886, 234.322,
    //     500,     669.009, 500,     330.991, 500);
    // cout << x << endl;
    // points2d.push_back(x);

    Mat points3d;
    triangulatePoints(points2d, Ps, points3d);
    cout << points3d.cols << endl;
    for (int i = 0; i < points3d.cols; i += 1)
    {
        cout << points3d.at<double>(0, i) << " "
             << points3d.at<double>(1, i) << " "
             << points3d.at<double>(2, i) << endl;
    }
}

int
main()
{
    vector<Mat> points2d;
    vector<Mat> Rs;
    vector<Mat> Ts;
    vector<Mat> points3d_estimated;


//    init_points(points2d);
    init_synth_points(points2d);
    triangulate(points2d);

    // Matx33d K = Matx33d(500, 0, 500,
    //                     0, 500, 500,
    //                     0, 0,  1);
    // reconstruct(InputArrayOfArrays(points2d), Rs, Ts, K, points3d_estimated, true);


    // for (int i = 0; i < points3d_estimated.size(); i++)
    // {
    //     auto p = points3d_estimated[i];

    //     for (int j = 0; j < 3; j++)
    //     {
    //         std::cout << std::fixed;
    //         std::cout << std::setprecision(10);
    //         cout << p.row(j).at<double>(0) << " ";
    //     }
    //     cout << endl;
    // }

    // cout << K << endl;

    return 0;
}
