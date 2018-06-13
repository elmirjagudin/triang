#include <iostream>
#include <opencv2/calib3d.hpp>
#include <opencv2/sfm.hpp>

#include "common.h"

#include "triangulate.h"
#include "show.h"


using namespace std;
using namespace cv;
using namespace cv::sfm;

const Matx33d K = Matx33d(FX, 0, CX,
                          0, FY, CY,
                          0, 0,  1);

static void
rt_vects(int i, Mat & rvec, Mat & tvec)
{
    auto yrot = ((float)i) / 4;
    auto t = ((float)i) / 3;

    rvec = (Mat_<double>(3,1) << 0, yrot, 0);
    tvec = (Mat_<double>(3,1) << t, 0, 0);
}

static void
filter_outliers(vector<Mat> & points2d, Mat & mask)
{
    Mat left = Mat_<double>(2, 0);
    Mat right = Mat_<double>(2, 0);

    int inliers = 0;
    for (int i = 0; i < mask.rows; i += 1)
    {
        if (!mask.at<bool>(i, 0))
        {
            continue;
        }
        inliers += 1;

        hconcat(left, points2d[0].col(i), left);
        hconcat(right, points2d[1].col(i), right);
    }

    points2d.clear();

    points2d.push_back(left);
    points2d.push_back(right);
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

    /* P2x from matches */
    auto pp = Point2d(CX, CY);
    Mat E = findEssentialMat(points2d[1].t(), points2d[0].t(),
                             FX, pp, RANSAC, 0.999, 0.5, mask);

    filter_outliers(points2d, mask);

    Mat local_R, local_t;
    recoverPose(E, points2d[1].t(), points2d[0].t(), local_R, local_t, FX, pp);
    cout << "E " << E << endl;
    cout << "local_R\n" << local_R << "\n local_t\n" << local_t << endl;
    cout << "R\n" << R << endl;

    Rodrigues(local_R, rvec);
    cout << "rvec" << rvec << endl;

    Mat P2x;
    projectionFromKRt(K, local_R, local_t, P2x);

    cout << "P1 " << P1 << endl;
    cout << "P2x " << P2x << endl;
    vector<Mat> Ps;
    Ps.push_back(P1);
    Ps.push_back(P2x);

    Mat points3d;
    triangulatePoints(points2d, Ps, points3d);

    show_point_cloud(points3d);
}