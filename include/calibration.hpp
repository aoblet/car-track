#include "opencv2/opencv.hpp"

void calibrateCamera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs);
void estimateMarkersPosition(cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs);