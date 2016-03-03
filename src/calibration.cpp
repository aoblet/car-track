#include "calibration.hpp"
#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include <vector>
#include <glm/vec3.hpp>

using namespace cv;

void calibrateCamera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs){
    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;

    cv::Mat inputImage;
    cv::Size imgSize((int)inputVideo.get(cv::CAP_PROP_FRAME_WIDTH), (int)inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(7, 5, 1, 0.5, dictionary);


    std::vector<std::vector<std::vector<cv::Point2f>>> allCornersConcatenated;
    std::vector<std::vector<int>> allIdsConcatenated;
    std::vector<cv::Mat> allImgs;
    bool rHasPressed = false;

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

        imshow("plop", fakeImage);

        if(key == 'r')
            rHasPressed = !rHasPressed;

        // if record
        if(rHasPressed){
            allImgs.push_back(inputImage);
            std::vector<int> markerIds;
            std::vector<std::vector<Point2f>> markerCorners;

            cv::aruco::detectMarkers(inputImage, board->dictionary, markerCorners, markerIds);

            // if at least one marker detected
            if(markerIds.size() > 0) {
                allCornersConcatenated.push_back(markerCorners);
                allIdsConcatenated.push_back(markerIds);
            }
            DLOG(INFO) << "Calibration record image DONE";

        }
        // if quit
        if(key == 27)
            break;
    }


    std::vector<Mat> allCorners;
    std::vector<Mat> allCharucoIds;
    for(size_t i=0; i<allIdsConcatenated.size(); ++i){
        Mat currentCharucoCorners, currentCharucoIds;
        aruco::interpolateCornersCharuco(allCornersConcatenated[i], allIdsConcatenated[i], allImgs[i], board,
                                         currentCharucoCorners, currentCharucoIds, cameraMatrix,
                                         distCoeffs);

        allCorners.push_back(currentCharucoCorners);
        allCharucoIds.push_back(currentCharucoIds);
    }
    std::vector<Mat> rvecs, tvecs;
    // After capturing in several viewpoints, start calibration
    int calibrationFlags = CALIB_FIX_ASPECT_RATIO;
    double repError = cv::aruco::calibrateCameraCharuco(allCorners, allCharucoIds, board, imgSize, cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);
    DLOG(INFO) << "After camera calibration: repError " << repError;
}

void estimateMarkersPosition(cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs){
    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);

    while (inputVideo.grab()) {
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        // if at least one marker detected
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distortionCoeffs, rvecs, tvecs);
            // draw axis for each marker
            for(size_t i=0; i<ids.size(); i++)
                cv::aruco::drawAxis(imageCopy, cameraMatrix, distortionCoeffs, rvecs[i], tvecs[i], 0.1);

//            for(auto& pos : tvecs)
//                DLOG(INFO) << pos;
        }

        cv::imshow("out", imageCopy);
        char key = (char) cv::waitKey(1);

        if (key == 27)
            break;
    }
}

void getMarkersPositionsPerFrame(cv::Mat& imageInput, cv::Ptr<cv::aruco::Dictionary>& dicMarkers,
                                 std::vector<int>& ids, std::vector<cv::Vec3d>& positions,
                                 cv::Mat& cameraMatrix, cv::Mat& distortionCoeffs, bool drawAxis){
    ids.clear();
    positions.clear();
    std::vector<std::vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(imageInput, dicMarkers, corners, ids);

    // if at least one marker detected
    if (ids.size() == 0)
        return;
    cv::aruco::drawDetectedMarkers(imageInput, corners, ids);
    std::vector<cv::Vec3d> rvecs;
    cv::aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distortionCoeffs, rvecs, positions);

    if(!drawAxis)
        return;
    
    // draw axis for each marker
    for(size_t i=0; i<ids.size(); i++)
        cv::aruco::drawAxis(imageInput, cameraMatrix, distortionCoeffs, rvecs[i], positions[i], 0.1);
}