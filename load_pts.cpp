#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "load_pts.h"

#define FP_PREFIX "/home/boris/Desktop/RectifiedImgs/CameraRig/cb0/"
#define WIN_NAME "loaded points"

static void
set_point(Mat_<double> & frame, int point_num, double x, double y)
{
//    cout << x << ", " << y << endl;
    frame(0, point_num) = x;
    frame(1, point_num) = y;
}

void
load_json(string name, Mat_<double> & points)
{
    string img_file = FP_PREFIX + name + ".png";
    string json_file = FP_PREFIX + name + ".json";

    cout << img_file << endl;
    cout << json_file << endl;

    Mat img = imread(img_file);

    Json::Value val;
    Json::Reader reader;

    ifstream jsonfile(json_file);
    if (!reader.parse(jsonfile, val))
    {
        cout << "Error: " << reader.getFormattedErrorMessages();
        return;
    }

    auto color = Scalar(255, 100, 255);
    auto json_pts = val["points"];
    points = Mat_<double>(2, json_pts.size());
    for (int i = 0; i < json_pts.size(); i += 1)
    {
        auto pt = Point(json_pts[i]["x"].asInt(),
                        json_pts[i]["y"].asInt());

        circle(img, pt, 3, color, 3);
        set_point(points, i, (double)pt.x, (double)pt.y);
    }

    // namedWindow(WIN_NAME, 1);
    // imshow(WIN_NAME, img);

    // waitKey(0);
}

void
load_points(vector<Mat> & points2d)
{
    Mat_<double> frame;

    load_json("img75", frame);
    points2d.push_back(Mat(frame));

    load_json("img112", frame);
    points2d.push_back(Mat(frame));

}