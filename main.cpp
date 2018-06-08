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
#include <stdlib.h>


using namespace std;
using namespace cv;
using namespace cv::sfm;

/* camera rig */
#define FX  668.0149082545821
#define FY  667.7009161134772
#define CX  612.5079400457761
#define CY  607.8048501338819
//LEFT:
//[664.3603430180929, 0, 615.4154619627029;
// 0, 664.3603430180929, 600.607663161244;
// 0, 0, 1]

//RIGHT:
//[668.0149082545821, 0, 612.5079400457761;
// 0, 667.7009161134772, 607.8048501338819;
// 0, 0, 1]

// #define F   655.899779
// #define CX  657.042298
// #define CY  614.458172


// #define F   664.383695213819
// #define CX  609.7365486001763
// #define CY  600.5922443696466

/* synthetic camera */
// #define F   500
// #define CX  500
// #define CY  500


const Matx33d K = Matx33d(FX, 0, CX,
                          0, FY, CY,
                          0, 0,  1);


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


//     /* img16.png */
//     frame = Mat_<double>(2, NUM_POINTS);
//     set_point(frame, B1, 585,  658);

//     set_point(frame, C1, 220,  675);
//     set_point(frame, C2, 420,  614);
//     set_point(frame, C3, 867,  704);
//     set_point(frame, C4, 916,  856);

//     set_point(frame, M1, 411,  987);
//     set_point(frame, M2, 555,  948);
//     set_point(frame, M3, 624, 1074);
//     set_point(frame, M4, 451, 1143);

// show_points("img16", frame);
//     points2d.push_back(Mat(frame));

//     /* img20.png */
//     frame = Mat_<double>(2, NUM_POINTS);
//     set_point(frame, B1, 671,  666);

//     set_point(frame, C1, 325,  674);
//     set_point(frame, C2, 508,  620);
//     set_point(frame, C3, 969,  721);
//     set_point(frame, C4, 1029, 891);

//     set_point(frame, M1, 494,  984);
//     set_point(frame, M2, 636,  957);
//     set_point(frame, M3, 705, 1094);
//     set_point(frame, M4, 532, 1142);

// show_points("img20", frame);
//     points2d.push_back(Mat(frame));
}

#define IMG_WIN "synth points"

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

    namedWindow(IMG_WIN, WINDOW_AUTOSIZE );
    imshow(IMG_WIN, img );
    waitKey(0);
//    destroyWindow(IMG_WIN);
}

#define Z_DIST 20

static void
rt_vects(int i, Mat & rvec, Mat & tvec)
{
    auto yrot = ((float)i) / 4;
    auto t = ((float)i) / 3;

    rvec = (Mat_<double>(3,1) << 0, yrot, 0);
    tvec = (Mat_<double>(3,1) << t, 0, 0);

    cout << " i " << i << endl
         << " rvec " << rvec << endl
         << " tvec " << tvec << endl;
}

static void
synth(int i, Mat & imagePoints)
{
    vector<Point3f> points;

    for (int j = 0; j < 6; j += 3)
    {
        // points.push_back(Point3f(0, 0, j+Z_DIST-10));
        // points.push_back(Point3f(0, 5, j+Z_DIST-5));
        // points.push_back(Point3f(5, 0, j+Z_DIST-5));
        // points.push_back(Point3f(0, -5, j+Z_DIST-5));
        // points.push_back(Point3f(-5, 0, j+Z_DIST-5));

        for (int i = -5; i <= 5; i += 1)
        {
            points.push_back(Point3f((float)i, 5, j+Z_DIST));
            points.push_back(Point3f((float)i, -5,j+Z_DIST));
        }

        for (int i = -4; i <= 4; i += 1)
        {
            points.push_back(Point3f(5, (float)i, j+Z_DIST));
            points.push_back(Point3f(-5, (float)i, j+Z_DIST));
        }
    }

    // for (int i = 0; i < points.size(); i += 1)
    // {
    //     auto p = points[i];
    //     cout << p.x << " " << p.y << " " << p.z << endl;
    // }

    // cout << "yrot " << yrot << " t " << t << endl;

    Mat rvec, tvec;
    rt_vects(i, rvec, tvec);

    Mat dist;

    vector<Point3f> sliced_points;
    for (int i = 0; i < 40; i += 1)
    {
        sliced_points.push_back(points[i]);
    }

    projectPoints(sliced_points, rvec, tvec, K, noArray(), imagePoints);
    show_synth_points(imagePoints);
}

void
init_synth_points(vector<Mat> & points2d)
{
    for (int i = -1;
         i <= 1; //i <= 6;
         i += 2)
    {
        Mat imagePoints;

        // auto yr = ((float)i) / 4;
        // auto t = ((float)i) / 3;
        synth(i, imagePoints);


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

void
show_point_cloud(Mat & points3d)
{
//    Mat cloud(3, points3d.cols, CV_64FC3);
    Mat cloud(points3d.cols, 1, CV_64FC3);

    for (int i = 0; i < points3d.cols; i += 1)
    {
        auto x = points3d.at<double>(0, i);
        auto y = points3d.at<double>(1, i);
        auto z = points3d.at<double>(2, i);

        // cout << x << " "
        //      << y << " "
        //      << z << endl;

        cloud.at<double>(i, 0) = x;
        cloud.at<double>(i, 1) = y;
        cloud.at<double>(i, 2) = z;
    }

    cv::viz::Viz3d myWindow("Viz Demo");
    auto cloud_widget = cv::viz::WCloud(cloud);
    myWindow.showWidget( "Depth", cloud_widget );

    myWindow.spin();
}

void
triangulate(vector<Mat> & points2d)
{
    Mat mask;
    Mat P1, P2, P3;
    Mat R;
    Mat rvec;
    Mat tvec;

    /* projection matrix P1 */
    rt_vects(0, rvec, tvec);
    Rodrigues(rvec, R);
    projectionFromKRt(K, R, tvec, P1);

    /* projection matrix P2 */
    rt_vects(1, rvec, tvec);
    Rodrigues(rvec, R);
    projectionFromKRt(K, R, tvec, P2);

    /* P2x from matches */
    auto pp = Point2d(CX, CY);
    Mat E = findEssentialMat(points2d[0].t(), points2d[1].t(),
                             FX, pp, RANSAC, 0.999, 0.5, mask);
    Mat local_R, local_t;
    recoverPose(E, points2d[0].t(), points2d[1].t(), local_R, local_t, FX, pp, mask);
    cout << "E " << E << endl;
    cout << "local_R\n" << local_R << "\n local_t\n" << local_t << endl;
    cout << "R\n" << R << endl;

    Rodrigues(local_R, rvec);
    cout << "rvec" << rvec << endl;

    Mat P2x;
    projectionFromKRt(K, local_R, local_t, P2x);

//     /* P3x from matches */
// cout << "------------- P3x ---------------\n";
//     E = findEssentialMat(points2d[2].t(), points2d[0].t(),
//                          500, pp, RANSAC, 0.999, 1.0, mask);

//     recoverPose(E, points2d[2].t(), points2d[0].t(), local_R, local_t, 500, pp, mask);
//     cout << "E " << E << endl;
//     cout << "local_R\n" << local_R << "\n local_t\n" << local_t * 2 << endl;
//     cout << "R\n" << R << endl;

//     Mat P3x;
//     projectionFromKRt(K, local_R, local_t * 2, P3x);

    cout << "P1 " << P1 << endl;
    cout << "P2 " << P2 << endl;
    cout << "P2x " << P2x << endl;
//    cout << "P3x " << P3x << endl;
    vector<Mat> Ps;
    Ps.push_back(P1);
    Ps.push_back(P2x);
//    Ps.push_back(P3x);

    Mat points3d;
    triangulatePoints(points2d, Ps, points3d);

    show_point_cloud(points3d);
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
