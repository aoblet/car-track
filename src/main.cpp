#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "calibration.hpp"

int main(int argc, char** argv) {
    cv::Mat cameraMatrix(cv::Mat::eye(3,3, CV_32F)), distCoeffs;
    DLOG(INFO) << "Matrix before calib " << cameraMatrix;
    calibrateCamera(cameraMatrix, distCoeffs);
    DLOG(INFO) << "Matrix after calib " << cameraMatrix;
    DLOG(INFO) << "Dist coeffs after calib " << distCoeffs;


    DLOG(INFO) << "Start display markers positions...";
    estimateMarkersPosition(cameraMatrix, distCoeffs);
    return 0;
}