#include <iostream>
#include <opencv2/viz.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "show.h"

using namespace cv;

static Mat img_matches;
static int highlight_point = -1;

static void
mouse_event_cb(int event, int x, int y, int flags, void* userdata)
{
    if (event != EVENT_LBUTTONDOWN)
    {
        /* not a mouse click event, ignore */
        return;
    }

    vector<Mat> *points2d = (vector<Mat> *)userdata;
    Mat lpts = points2d->at(0);

    bool found_pt = false;

    Mat pt;

    for (int i = 0; i < lpts.cols; i += 1)
    {
        pt = lpts.col(i);
        auto click = Matx21d((double)x, (double)y);

        /* norm gives eculidian distance */
        if (norm(pt, click) < 5.0)
        {
            highlight_point = i;
            found_pt = true;
            break;
        }
    }

    if (!found_pt)
    {
        highlight_point = -1;
        return;
    }

    circle(img_matches,
           Point(pt.at<double>(0, 0),
                 pt.at<double>(1, 0)),
           3, Scalar::all(-1), 3);
    imshow("matches", img_matches);
}

void
show_points(Mat & left_img, Mat & right_img, vector<Mat> &points2d)
{
    Mat left = points2d[0];
    Mat right = points2d[1];

    assert(left.cols == right.cols);

    /* use drawMatches to create left and right side-by-side image */
    vector<KeyPoint> key_pts_l, key_pts_r;
    vector<DMatch> matches;
    drawMatches(left_img, key_pts_l,
                right_img, key_pts_r,
                matches, img_matches);

    auto lcolor = Scalar(255, 100, 255);
    auto rcolor = Scalar(255, 100, 0);
    auto img_width = left_img.size().width;

    for (int i = 0; i < left.cols; i += 1)
    {
         Point pt = Point(left.at<double>(0, i),
                          left.at<double>(1, i));

         circle(img_matches, pt, 3, lcolor, 3);

         pt = Point(img_width + right.at<double>(0, i),
                    right.at<double>(1, i));

         circle(img_matches, pt, 3, rcolor, 3);
    }

    namedWindow("matches", 1);
    imshow("matches", img_matches);
}

void
update_cloud_widget(cv::viz::Viz3d & win, Mat & points3d)
{
    static bool toggle = true;
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
    Mat colors = Mat(cloud.size(), CV_8UC3, cv::viz::Color::green());
    if (highlight_point != -1)
    {
        colors.row(highlight_point) = toggle ? cv::viz::Color::white() : cv::viz::Color::black();
        toggle = !toggle;
    }

    auto cloud_widget = cv::viz::WCloud(cloud, colors);

    //win.removeWidget("Depth");
    win.showWidget("Depth", cloud_widget );
}

void
show_point_cloud(Mat & left_img, Mat & right_img, vector<Mat> & points2d, Mat & points3d)
{
    show_points(left_img, right_img, points2d);

    cv::viz::Viz3d win("Viz Demo");
    win.setBackgroundColor(cv::viz::Color::black());
    setMouseCallback("matches", mouse_event_cb, &points2d);

    update_cloud_widget(win, points3d);

    while (true)
    {
        win.spinOnce();
        waitKey(500);

        if (highlight_point != -1)
        {
            update_cloud_widget(win, points3d);
        }
    }
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
}