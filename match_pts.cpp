//#include <opencv2/core.hpp>
//#include "opencv2/imgproc.hpp"
//#include <opencv2/features2d/features2d.hpp>

#include <opencv2/viz.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include "common.h"

#include "match_pts.h"

string image_path(string name)
{
    return
       "/home/boris/Desktop/RectifiedImgs/CameraRig/pic0/" + name + ".png";
}

static void
set_point(Mat_<double> & frame, int point_num, double x, double y)
{
//    cout << x << ", " << y << endl;
    frame(0, point_num) = x;
    frame(1, point_num) = y;
}

static void
get_keypoints(Mat & frame, vector<KeyPoint> & keypoints, Mat & descriptors)
{
    static auto orb = ORB::create();

    orb->detectAndCompute(_InputArray(frame), cv::noArray(), keypoints, descriptors);
}

static Mat image0, image20;

void load_image(string name, Mat & frame)
{
    auto tmp =
        imread(image_path(name), CV_LOAD_IMAGE_COLOR);

    resize(tmp, frame, cv::Size(), DOWNSAMPLE, DOWNSAMPLE);
}

void
match_points(vector<Mat> &points2d)
{
    auto orb = ORB::create(2000);
    vector<KeyPoint> keypoints0, keypoints20;
    Mat descriptors0, descriptors20;

    load_image("img0", image0);
    load_image("img16", image20);

    get_keypoints(image0, keypoints0, descriptors0);
    get_keypoints(image20, keypoints20, descriptors20);

    static auto matcher = BFMatcher(NORM_HAMMING, true);
    vector<DMatch> matches;
                  /* query */   /* train */
    matcher.match(descriptors0, descriptors20, matches);

    // Mat img_matches;
    // drawMatches(image0, keypoints0,
    //             image20, keypoints20,
    //             matches, img_matches);

    // namedWindow("matches", 1);
    // imshow("matches", img_matches);
    // waitKey(0);

    auto pts0 = Mat_<double>(2, 0);
    auto pts20 = Mat_<double>(2, 0);
    for (int i = 0; i < matches.size(); i += 1)
    {
        auto match = matches[i];
        if (match.distance > MATCH_DISTANCE)
        {
            /* skip */
            continue;
        }

        auto pt = keypoints0[match.queryIdx].pt;

        Mat mpt = (Mat_<double>(2, 1) << pt.x, pt.y);
        hconcat(pts0, mpt, pts0);
//        set_point(pts0, i, pt.x, pt.y);

        pt = keypoints20[match.trainIdx].pt;
        mpt = (Mat_<double>(2, 1) << pt.x, pt.y);
        hconcat(pts20, mpt, pts20);

//        set_point(pts20, i, pt.x, pt.y);
    }

    cout << pts0 << endl;

//        hconcat(left, points2d[0].col(i), left);
//        hconcat(right, points2d[1].col(i), right);


    points2d.push_back(pts0);
    points2d.push_back(pts20);
}

void
show_matches(vector<Mat> &points2d)
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

    drawMatches(image0, key_pts_l,
                image20, key_pts_r,
                matches, img_matches);

    namedWindow("matches", 1);
    imshow("matches", img_matches);

    while (waitKey(0) != 27)
    {
        /* nop */
    }
}