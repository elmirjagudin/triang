#include <iostream>
#include <opencv2/viz.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "show.h"

using namespace cv;

void
show_point_cloud(Mat & points3d)
{
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
show_matches(Mat & left_img, Mat & right_img, vector<Mat> &points2d)
{
    Mat img_matches;

    vector<KeyPoint> key_pts_l, key_pts_r;
    vector<DMatch> matches;

    Mat left = points2d[0];
    Mat right = points2d[1];

    cout << left.cols << " < l vs r > " << right.cols << endl;

    assert(left.cols == right.cols);
    for (int i = 0; i < left.cols; i += 1)
    {
        auto kp = KeyPoint();
        kp.pt.x = left.at<double>(0, i);
        kp.pt.y = left.at<double>(1, i);
        key_pts_l.push_back(kp);

        kp = KeyPoint();
        kp.pt.x = right.at<double>(0, i);
        kp.pt.y = right.at<double>(1, i);
        key_pts_r.push_back(kp);

        matches.push_back(DMatch(i, i, 0));

    }

    drawMatches(left_img, key_pts_l,
                right_img, key_pts_r,
                matches, img_matches);

    namedWindow("matches", 1);
    imshow("matches", img_matches);

    while (waitKey(0) != 27)
    {
        /* nop */
    }
}