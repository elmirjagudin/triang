#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

#include <opencv2/sfm.hpp>
#include <opencv2/viz.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "common.h"

#include "match_pts.h"
#include "load_pts.h"
#include "triangulate.h"
#include "show.h"


using namespace std;
using namespace cv;
using namespace cv::sfm;

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
        "/home/boris/Desktop/RectifiedImgs/CameraRig/cb0/" + fname + ".png";

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
show_points("img18", frame);
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

int
main()
{
    vector<Mat> points2d;
    Mat points3d;
    Mat left_img, right_img;

    match_points(left_img, right_img, points2d);
//    load_points(points2d);
//    init_points(points2d);
//    init_synth_points(points2d);
    triangulate(points2d, points3d);

    show_point_cloud(left_img, right_img, points2d, points3d);

    return 0;
}
