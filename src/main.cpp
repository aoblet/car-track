#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include "calibration.hpp"
#include "trailManager.hpp"

int main(int argc, char** argv) {
    cv::Mat cameraMatrix(cv::Mat::eye(3,3, CV_32F)), distCoeffs;
    DLOG(INFO) << "Matrix before calib " << cameraMatrix;
    calibrateCamera(cameraMatrix, distCoeffs);
    DLOG(INFO) << "Matrix after calib " << cameraMatrix;
    DLOG(INFO) << "Dist coeffs after calib " << distCoeffs;


    DLOG(INFO) << "Start display markers positions...";
    estimateMarkersPosition(cameraMatrix, distCoeffs);

    // get corner positions
    std::vector<glm::vec3> corners;
    //TODO

    //create TrailManager
    int finalImgWidth = 800; // ???
    int finalImgHeight = 600; // ???
    TrailManager trailManager(finalImgWidth, finalImgHeight, 2, 100, 5);
    trailManager.updateCameraPos(corners, 10); //update the position of the camera based on the 4 corners and a height.
    //the final image with trails render to it
    cv::Mat trailImg(finalImgWidth, finalImgHeight, CV_8UC3);


    //TODO something like that.....
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
            //optional :
            //cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);

            //get marker positions :
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);

            //for each marker :
            for(size_t i=0; i<ids.size(); i++)
            {
                //Optionnal :
                //cv::aruco::drawAxis(imageCopy, cameraMatrix, distortionCoeffs, rvecs[i], tvecs[i], 0.1);

                //need marker ids + markers translations, update trails :
                trailManager.updateFromOpenCV(ids, tvecs);
            }
            //render the trails :
            trailManager.renderToTexture();

            //TODO : get the opengl texture and give it to openCV, modify trailImg
            trailManager.convertGlTexToCVMat(trailImg);
            //the result img may be flipped :
            // cv::flip(trailImg, trailImgFlipped, 0);

            //TODO : opencv mapping to render the trails


        }

        //display the final image :
        cv::imshow("out", trailImg);

        char key = (char) cv::waitKey(1);

        if (key == 27)
            break;
    }
    //......



    return 0;
}
