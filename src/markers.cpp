#include "markers.hpp"
#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include <vector>
#include <glm/vec3.hpp>

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
        }

        cv::imshow("out", imageCopy);
        char key = (char) cv::waitKey(1);

        if (key == 27)
            break;
    }
}

void getMarkersPositionsPerFrameWorld(cv::Mat &imageInput, cv::Ptr<cv::aruco::Dictionary> &dictMarkers,
                                      std::vector<int> &ids, std::vector<cv::Vec3d> &positions,
                                      cv::Mat &cameraMatrix, cv::Mat &distortionCoeffs, bool drawAxis){
    ids.clear();
    positions.clear();
    std::vector<std::vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(imageInput, dictMarkers, corners, ids);

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

bool getBordersPositionsWorld(cv::Mat &imageInput, cv::Ptr<cv::aruco::Dictionary> &dictMarkers,
                              std::vector<cv::Vec3d> &positions, cv::Mat &cameraMatrix, cv::Mat &distortionCoeffs,
                              bool drawAxis) {
    // bordersIds = {0,1,2,3};
    positions.clear();
    std::vector<int> ids;
    std::vector<cv::Vec3d> posMarkers;

    getMarkersPositionsPerFrameWorld(imageInput, dictMarkers, ids, posMarkers, cameraMatrix, distortionCoeffs, true);
    if(ids.size() > 0){
        for(unsigned int i=0; i <ids.size(); ++i){
            if(ids[i] >= 0 && ids[i]< 4)
                positions.push_back(posMarkers[i]);
        }
    }
    return positions.size() == 4;
}

void createMarkers(int nb, int size){
    cv::Ptr<cv::aruco::Dictionary> dictionaryy = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
    cv::Mat marker;

    for(int i=0; i<nb; ++i){
        cv::aruco::drawMarker(dictionaryy, i, size, marker, 1);
        cv::imwrite("aruco_"+std::to_string(i)+".png", marker);
    }
}