#include "opencv2/opencv.hpp"
#include <opencv2/aruco/dictionary.hpp>
void calibrateCamera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs);
void estimateMarkersPosition(cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs);
void getMarkersPositionsPerFrame(cv::Mat& imageInput, cv::Ptr<cv::aruco::Dictionary>& dicMarkers,
                                 std::vector<int>& ids, std::vector<cv::Vec3d>& positions,
                                 cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs, bool drawAxis=false);