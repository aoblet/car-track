#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include <vector>

using namespace cv;

void calibrateCamera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs){
    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;

    cv::Mat inputImage;
    cv::Size imgSize((int)inputVideo.get(cv::CAP_PROP_FRAME_WIDTH), (int)inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(7, 5, 1, 0.5, dictionary);


    std::vector<std::vector<cv::Point2f>> allCornersConcatenated;
    std::vector<std::vector<int>> allIdsConcatenated;

    while(1){
        char key = (char)cv::waitKey(1);
        inputVideo >> inputImage;
        cv::Mat fakeImage;
        inputImage.copyTo(fakeImage);

        std::vector<int> fakeIds;
        std::vector< std::vector<Point2f> > fakeCorners;
        cv::aruco::detectMarkers(fakeImage, board->dictionary, fakeCorners, fakeIds);

        if(fakeIds.size() > 0) {
            aruco::drawDetectedMarkers(fakeImage, fakeCorners, fakeIds);
        }

        cv::flip(fakeImage, fakeImage, 1);
        imshow("plop", fakeImage);

        // if record
        if(key == 'r'){
            std::vector<int> markerIds;
            std::vector<Point2f> markerCorners;

            DLOG(INFO) << "Calibration record image ..";
            cv::aruco::detectMarkers(inputImage, board->dictionary, markerCorners, markerIds);

            // if at least one marker detected
            if(markerIds.size() > 0) {
                allCornersConcatenated.push_back(markerCorners);
                allIdsConcatenated.push_back(markerIds);
            }
        }
        // if quit
        if(key == 27)
            break;
    }

    // After capturing in several viewpoints, start calibration
    std::vector< Mat > rvecs, tvecs;
    int calibrationFlags = CALIB_FIX_ASPECT_RATIO;
    double repError = cv::aruco::calibrateCameraCharuco(allCornersConcatenated, allIdsConcatenated, board, imgSize, cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);
    DLOG(INFO) << "After camera calibration: repError " << repError;
}

int main(int argc, char** argv) {
    cv::Mat cameraMatrix(cv::Mat::eye(3,3, CV_32F)), distCoeffs;
    DLOG(INFO) << "Matrix before calib " << cameraMatrix;
    calibrateCamera(cameraMatrix, distCoeffs);
    DLOG(INFO) << "Matrix after calib " << cameraMatrix;
    return 0;
}