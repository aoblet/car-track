#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imguiRenderGL3.h"

#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include "calibration.hpp"
#include "trailManager.hpp"
#include "ShaderProgram.hpp"
#include "markers.hpp"

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;

int main(int argc, char** argv) {
//    cv::Mat cameraMatrix(cv::Mat::eye(3,3, CV_32F)), distCoeffs;
//    DLOG(INFO) << "Matrix before calib " << cameraMatrix;
//    calibrateCamera(cameraMatrix, distCoeffs);
//
//    DLOG(INFO) << "Matrix after calib " << cameraMatrix;
//    DLOG(INFO) << "Dist coeffs after calib " << distCoeffs;
//
//    DLOG(INFO) << "Start trail engine stuff...";
//
//    cv::VideoCapture inputVideo(-1);
//    if(!inputVideo.open(-1))
//        throw;
//
//    const cv::Mat camToWorld = cameraMatrix.inv();
//    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
//
//    // get corner positions
//    std::vector<cv::Vec3d> positionsBorders;
//    while(inputVideo.grab()){
//        cv::Mat image, imageCopy;
//        inputVideo.retrieve(image);
//        image.copyTo(imageCopy);
//
//        if(getBordersPositionsWorld(imageCopy, dictionary, positionsBorders, cameraMatrix, distCoeffs, true)){
//            DLOG(INFO) << "Borders found";
////            break;
//        }
//        cv::imshow("out", imageCopy);
//
//        char key = (char) cv::waitKey(1);
//        if (key == 27)
//            break;
//    }



    int width = 1280, height= 720;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    int const DPI = 1;

    // Open a window and create its OpenGL context
    GLFWwindow * window = glfwCreateWindow(width/DPI, height/DPI, "car-track", 0, 0);
    if( ! window )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    glfwMakeContextCurrent(window);

    // Init glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit( EXIT_FAILURE );
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );
    GLenum glerr = GL_NO_ERROR;
    glerr = glGetError();

    if (!imguiRenderGLInit(DroidSans_ttf, DroidSans_ttf_len))
    {
        fprintf(stderr, "Could not init GUI renderer.\n");
        exit(EXIT_FAILURE);
    }

    // ---------------- Create shader that will draw quad

    Graphics::ShaderProgram quadProgram("../shaders/blit.vert", "", "../shaders/blit.frag");
    quadProgram.useProgram();

    // ---------------- Create quad that will contain render

    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3};

    std::vector<cv::Point2f> original = {
            cv::Point2f(-1.0, -1.0),
            cv::Point2f(1.0, -1.0),
            cv::Point2f(-1.0, 1.0),
            cv::Point2f(1.0, 1.0),
    };

    std::vector<glm::vec2> quadVertices = {
            glm::vec2(-1.0, -1.0),
            glm::vec2(1.0, -1.0),
            glm::vec2(-1.0, 1.0),
            glm::vec2(1.0, 1.0),
    };

    std::vector<glm::vec2> quadTexcoord = {
            glm::vec2(0, 0),
            glm::vec2(1, 0),
            glm::vec2(0, 1),
            glm::vec2(1, 1),
    };

    GLuint quadVAO, quadVBOVertices, quadVBOIndexes, quadVBOTexcoord;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBOVertices);
    glGenBuffers(1, &quadVBOIndexes);
    glGenBuffers(1, &quadVBOTexcoord);

    // Quad
    glBindVertexArray(quadVAO);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVBOIndexes);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);

    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, quadVBOVertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_DYNAMIC_DRAW);

    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, quadVBOTexcoord);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, quadTexcoord.size() * sizeof(glm::vec2), quadTexcoord.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Launch capture -------------------------------------------------------------------------------------------------------------------------------

    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;

    cv::Mat captureImage;


    // Create Texture -------------------------------------------------------------------------------------------------------------------------------

    int capWidth = inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
    int capHeight = inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

    GLuint cvTexture;
    glGenTextures(1, &cvTexture);
    glBindTexture(GL_TEXTURE_2D, cvTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, capWidth, capHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // GUI vars -------------------------------------------------------------------------------------------------------------------------------

    int logScroll = 0;
    float t = 0;
    float fps = 0.f;

    bool fullscreen = false;
    bool keypressed = false;

    do
    {
        glfwGetWindowSize(window, &width, &height);
        inputVideo >> captureImage;

        cv::flip(captureImage, captureImage, -1);

        glBindTexture(GL_TEXTURE_2D, cvTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, capWidth, capHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, captureImage.data);

        t = glfwGetTime();

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quadProgram.useProgram();

        // Find Homography -------------------------------------------------------------------------------------------------------------------------------

        cv::Mat homography;

        std::vector<cv::Point2f> stretched;

        for(auto& vert : quadVertices){
            stretched.push_back(cv::Point2f(vert.x, vert.y));
        }

        homography = cv::findHomography(original, stretched);

        glm::mat3 glHomography;

        for(int i = 0; i < homography.rows; ++i){
            for(int j = 0; j < homography.cols; ++j){
                glHomography[j][i] = float(homography.at<double>(i,j));
            }
        }

        quadProgram.updateUniform("Homography", glHomography);

        // Find Draw -------------------------------------------------------------------------------------------------------------------------------

        glViewport(0, 0, width, height);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBOVertices);
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_STATIC_DRAW);

        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, cvTexture);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        // Draw UI
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        unsigned char mbut = 0;
        int mscroll = 0;
        double mousex; double mousey;
        glfwGetCursorPos(window, &mousex, &mousey);
        mousex*=DPI;
        mousey*=DPI;
        mousey = height - mousey;

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
            mbut |= IMGUI_MBUT_LEFT;
        }

        imguiBeginFrame(mousex, mousey, mbut, mscroll);
        char lineBuffer[512];

        float xwidth = 300;
        float ywidth = 500;

        imguiBeginScrollArea("car-track", width - xwidth - 10, height - ywidth - 10, xwidth, ywidth, &logScroll);
        sprintf(lineBuffer, "FPS %f", fps);
        imguiLabel(lineBuffer);

        for(auto& vec : quadVertices){
            imguiSeparatorLine();
            imguiSlider("x", &vec.x, -1, 1, 0.01);
            imguiSlider("y", &vec.y, -1, 1, 0.01);
        }

        imguiEndScrollArea();

        imguiEndFrame();
        imguiRenderGLDraw(width, height);

        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();

        double newTime = glfwGetTime();
        fps = 1.f/ (newTime - t);

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keypressed){
            fullscreen = !fullscreen;
            keypressed = true;
            DLOG(INFO) << "SWITCH TO " << (fullscreen ? "FULLSCREEN" : "WINDOWED");

            GLFWwindow * tmpWindow = glfwCreateWindow(width, height, "salut", fullscreen ? glfwGetPrimaryMonitor() : NULL, window);
//            glfwDestroyWindow(window);
            window = tmpWindow;

            glfwMakeContextCurrent(window);
            glfwSwapInterval(1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && keypressed){
            keypressed = false;
        }

    } while(glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose(window));

    glfwTerminate();

//    //create TrailManager
//    int finalImgWidth = 800; // ???
//    int finalImgHeight = 600; // ???
//    TrailManager trailManager(finalImgWidth, finalImgHeight, 2, 100, 5);
//    trailManager.updateCameraPos(positionsBorders, 10); //update the position of the camera based on the 4 corners and a height.
//    //the final image with trails render to it
//    cv::Mat trailImg(finalImgWidth, finalImgHeight, CV_8UC3);
//
//    while (inputVideo.grab()) {
//        cv::Mat image, imageCopy;
//        inputVideo.retrieve(image);
//        image.copyTo(imageCopy);
//        std::vector<int> ids;
//        std::vector<cv::Vec3d> positions;
//        getMarkersPositionsPerFrameWorld(imageCopy, dictionary, ids, positions, cameraMatrix, distCoeffs, true);
//
//        // if at least one marker detected
//        if (ids.size() > 0) {
//            //get marker positions :
//            trailManager.updateFromOpenCV(camToWorld, ids, positions);
//
//            //render the trails :
//            trailManager.renderToTexture();
//
//            //TODO : get the opengl texture and give it to openCV, modify trailImg
//            trailManager.convertGlTexToCVMat(trailImg);
//            //the result img may be flipped :
//            // cv::flip(trailImg, trailImgFlipped, 0);
//
//            //TODO : opencv mapping to render the trails
//        }
//
//        //display the final image :
//        cv::imshow("out", trailImg);
//
//        char key = (char) cv::waitKey(1);
//
//        if (key == 27)
//            break;
//    }
    return 0;
}
