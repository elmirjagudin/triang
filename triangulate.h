#include <opencv2/core.hpp>

using namespace cv;
using namespace std;

extern const Matx33d K;

void
triangulate(vector<Mat> & points2d, Mat & points3d);