#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include "calibration.hpp"
#include "trailManager.hpp"
#include "markers.hpp"

int main(int argc, char** argv) {

    //FEW TESTS FOR TRAIL MANAGER WITH OPENGL :
    //testDrawFollowingMouse();
    testDrawToTexture();

    cv::Mat cameraMatrix(cv::Mat::eye(3,3, CV_32F)), distCoeffs;
    DLOG(INFO) << "Matrix before calib " << cameraMatrix;
    calibrateCamera(cameraMatrix, distCoeffs);

    DLOG(INFO) << "Matrix after calib " << cameraMatrix;
    DLOG(INFO) << "Dist coeffs after calib " << distCoeffs;

    DLOG(INFO) << "Start trail engine stuff...";

    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;

    const cv::Mat camToWorld = cameraMatrix.inv();
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);

    // get corner positions
    std::vector<cv::Vec3d> positionsBorders;
    while(inputVideo.grab()){
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);

        if(getBordersPositionsWorld(imageCopy, dictionary, positionsBorders, cameraMatrix, distCoeffs, true)){
            DLOG(INFO) << "Borders found";
//            break;
        }
        cv::imshow("out", imageCopy);

        char key = (char) cv::waitKey(1);
        if (key == 27)
            break;
    }


    //create TrailManager
    int finalImgWidth = 800; // ???
    int finalImgHeight = 600; // ???
    TrailManager trailManager(finalImgWidth, finalImgHeight, 2, 100, 5);
    trailManager.updateCameraPos(positionsBorders, 10); //update the position of the camera based on the 4 corners and a height.
    //the final image with trails render to it
    cv::Mat trailImg(finalImgWidth, finalImgHeight, CV_8UC3);

    while (inputVideo.grab()) {
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        std::vector<int> ids;
        std::vector<cv::Vec3d> positions;
        getMarkersPositionsPerFrameWorld(imageCopy, dictionary, ids, positions, cameraMatrix, distCoeffs, true);

        // if at least one marker detected
        if (ids.size() > 0) {
            //get marker positions :
            trailManager.updateFromOpenCV(camToWorld, ids, positions);

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

    return 0;
}
