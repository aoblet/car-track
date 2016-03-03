#include "opencv2/opencv.hpp"
#include <opencv2/aruco/dictionary.hpp>

void estimateMarkersPosition(cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs);
void getMarkersPositionsPerFrameWorld(cv::Mat &imageInput, cv::Ptr<cv::aruco::Dictionary> &dictMarkers,
                                      std::vector<int> &ids, std::vector<cv::Vec3d> &positions,
                                      cv::Mat &cameraMatrix, cv::Mat &distortionCoeffs, bool drawAxis = false);

bool getBordersPositionsWorld(cv::Mat &imageInput, cv::Ptr<cv::aruco::Dictionary> &dictMarkers,
                              std::vector<cv::Vec3d> &positions,
                              cv::Mat &cameraMatrix, cv::Mat &distortionCoeffs, bool drawAxis = false);

void createMarkers(int nb, int size);