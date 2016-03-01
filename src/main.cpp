#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
using namespace cv;

int main(int argc, char** argv) {
    VideoCapture cap(-1);

    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!cap.open(-1))
        return 0;

    while (1) {
        Mat frame;
        cap >> frame;
        if( frame.empty() ) break; // end of video stream
        imshow("this is you, smile! :)", frame);
        if( waitKey(1) > 0 ) break; // stop capturing by pressing ESC
    }
    return 0;
}