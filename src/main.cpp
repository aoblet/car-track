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
#include "timer.hpp"

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;

int main(int argc, char** argv) {

    //FEW TESTS FOR TRAIL MANAGER WITH OPENGL :
    //testDrawFollowingMouse();
    //testDrawToTexture();

    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
    std::vector<std::vector<cv::Point2f>> markersCorners;
    std::vector<int> markersIds;

//    cv::Mat frame = cv::imread("image.png");
//    getBordersScreenPositions(frame, dictionary, markersCorners, markersIds, true);
//    cv::imshow("out", frame);
//    char key = (char) cv::waitKey(0);

    while(inputVideo.grab()){
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);

        getBordersScreenPositions(imageCopy, dictionary, markersCorners, markersIds, true);

        cv::imshow("out", imageCopy);

        char key = (char) cv::waitKey(1);
        if (key == 27 && markersIds.size() == 4){
            break;
        }
    }

    float camWidth = inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
    float camHeight = inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

    DLOG(INFO) << "CAM_W : " << camWidth;
    DLOG(INFO) << "CAM_H : " << camHeight;

    std::vector<cv::Point2f> bordersPositions;
    std::vector<cv::Point2f> bordersPositionsOrdered;
    std::vector<cv::Point2f> bordersUvs;
    std::vector<cv::Point2f> bordersUvsOrdered;

    bordersPositionsOrdered.resize(4);
    bordersUvsOrdered.resize(4);

    getMarkersCenters(markersCorners, markersIds, bordersPositions);

    for(auto& bordersPos : bordersPositions){
        bordersUvs.push_back(cv::Point2f(bordersPos.x / camWidth, bordersPos.y/camHeight));
    }

    for(int i = 0; i < markersIds.size(); ++i){
        bordersPositionsOrdered[markersIds[i]] = bordersPositions[i];
        bordersUvsOrdered[markersIds[i]] = bordersUvs[i];
    }

    DLOG(INFO) << "UNORDERED : ";
    for(int i = 0; i < markersIds.size(); ++i){
        DLOG(INFO) << "ID:";
        DLOG(INFO) << markersIds[i];
        DLOG(INFO) << "BORDER POS:";
        DLOG(INFO) << bordersPositions[i].x << " " << bordersPositions[i].y;
        DLOG(INFO) << "BORDER UV:";
        DLOG(INFO) << bordersUvs[i].x << " " << bordersUvs[i].y;
    }

    bordersPositions = bordersPositionsOrdered;
    bordersUvs = bordersUvsOrdered;

    DLOG(INFO) << "-------------------------------------------------";
    DLOG(INFO) << "ORDERED : ";
    for(int i = 0; i < markersIds.size(); ++i){
        DLOG(INFO) << "ID:";
        DLOG(INFO) << i;
        DLOG(INFO) << "BORDER POS:";
        DLOG(INFO) << bordersPositions[i].x << " " << bordersPositions[i].y;
        DLOG(INFO) << "BORDER UV:";
        DLOG(INFO) << bordersUvs[i].x << " " << bordersUvs[i].y;
    }

    std::vector<cv::Point2f> bordersTargetPositions = {
            cv::Point2f(-1.0f, +1.0f), // id = 0
            cv::Point2f(-1.0f, -1.0f), // id = 1
            cv::Point2f(+1.0f, -1.0f), // id = 2
            cv::Point2f(+1.0f, +1.0f), // id = 3
    };

    std::vector<cv::Point2f> bordersTargetUvs = {
            cv::Point2f(0, 1), // id = 0
            cv::Point2f(0, 0), // id = 1
            cv::Point2f(1, 0), // id = 2
            cv::Point2f(1, 1), // id = 3
    };

    glm::mat3 glBordersHomography;
    cv::Mat cvBordersHomography;

    glm::mat3 glBordersHomography2;
    cv::Mat cvBordersHomography2;


    bool inverse = true;

    cvBordersHomography = cv::findHomography(bordersTargetPositions, bordersPositions);
    glBordersHomography = convertCVMatrix3x3(cvBordersHomography);

    cvBordersHomography2 = cv::findHomography(bordersUvs, bordersTargetUvs);
    glBordersHomography2 = convertCVMatrix3x3(cvBordersHomography2);


    DLOG(INFO) << "HOMOGRAPHY";
    for(auto& point : bordersUvs){
        glm::vec3 toto(glBordersHomography2 * glm::vec3(point.x, point.y,1));
        toto/=toto.z;
        DLOG(INFO) << point << " TO " << glm::to_string(toto);
    }

    DLOG(INFO) << "HOMOGRAPHY_INV";
    for(auto& point : bordersTargetUvs){
        glm::vec3 toto(glm::inverse(glBordersHomography2) * glm::vec3(point.x, point.y,1));
        toto/=toto.z;
        DLOG(INFO) << point << " TO " << glm::to_string(toto);
    }

//    return 0;

    //set window size :
    int width = 640, height= 480;
    //set corners position :
    std::vector<cv::Vec3d> corners = {cv::Vec3d(width,height,0), cv::Vec3d(0,height,0), cv::Vec3d(0,0,0), cv::Vec3d(width,0,0)};

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

//    cv::VideoCapture inputVideo(1);
//    if(!inputVideo.open(1))
//        throw;

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

    // Create TrailManager --------------------------------------------------------------------------------------------------------------------

    int finalImgWidth = width; // ???
    int finalImgHeight = height; // ???
    TrailManager trailManager(finalImgWidth, finalImgHeight, 1, 128, 10);
    trailManager.updateCameraPos(corners, 10);
    //cv::Mat trailImg(finalImgHeight, finalImgWidth, CV_8UC3);
    Timer trailTimer;

    // Set up Aruco -------------------------------------------------------------------------------------------------------------
//    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(7, 5, 1, 0.5, dictionary);

    do
    {
        // GLFW inputs ---------------------------------------------------------------------------------------------------------
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



        // get camera image ---------------------------------------------------------------------------
        glfwGetWindowSize(window, &width, &height);
        inputVideo >> captureImage;

//        cv::flip(captureImage, captureImage, 0);

        //--------------------------------------------------------------------------------------------------------

        //Get marker positions ------------------------------------------------------------------------------------
        std::vector<int> fakeIds;
        std::vector< std::vector<cv::Point2f> > markerCorners;
        cv::aruco::detectMarkers(captureImage, board->dictionary, markerCorners, fakeIds);
        //markerCenters will contains the center of each marker.
        std::vector<glm::vec2> markerCenters;
        for(int i = 0; i < markerCorners.size(); i++)
        {
            //get center of each corner :
            glm::vec2 sum(0,0);
            for(int j = 0; j < markerCorners[i].size(); j++)
            {
                sum += glm::vec2(markerCorners[i][j].x, markerCorners[i][j].y);
            }
            sum /= markerCorners[i].size();

            markerCenters.push_back(sum);
            std::cout<<"marker detected at position : ("<<markerCenters.back().x<<", "<<markerCenters.back().y<<")"<<std::endl;
        }

        // Draw in cvTexture ---------------------------------------------------------------------------------------

        if(fakeIds.size() > 0) {
            cv::aruco::drawDetectedMarkers(captureImage, markerCorners, fakeIds);
        }

        glBindTexture(GL_TEXTURE_2D, cvTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, capWidth, capHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, captureImage.data);

        // ------------------------------------------------------------------------------------------------------------


        //update trails ------------------------------------------------------------------------------------------------------
        //debug with mouse :
//        if(mbut & IMGUI_MBUT_LEFT)
//        {
//            std::cout<<mousex<<", "<<mousey<<std::endl;
//            if(trailTimer.elapsedTime() > 0.05f)
//            {
//                std::cout<<"add point to trail"<<std::endl;
//                trailManager.getTrail(0).pushBack(glm::vec3(mousex, -mousey+height, 0), glm::vec3(1,0,0));
//                trailTimer.restart();
//            }
//        }
        //add point to trail base on the first marker position :
        if(trailTimer.elapsedTime() > 0.003f && markerCenters.size() >0)
        {
            float markerX = (markerCenters[0].x / capWidth)*width;
            float markerY =  height-(markerCenters[0].y / capHeight)*height;

            std::cout<<"add point to trail at position : ("<<markerX<<", "<<markerY<<")"<<std::endl;
            trailManager.getTrail(0).pushBack(glm::vec3(markerX, markerY, 0), glm::vec3(1,0,0));
            trailTimer.restart();
        }

        // -------------------------------------------------------------------------------------------

        //render trails ------------------------------------------------------------------------------

        trailManager.updateTrails();
        trailManager.synchronizeVBOTrails();
        //trailManager.renderToTexture();
        trailManager.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //draw background :
            quadProgram.useProgram();
            glBindBuffer(GL_ARRAY_BUFFER, quadVBOVertices);
            glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_STATIC_DRAW);
            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, cvTexture);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        trailManager.renderTrails();
        trailManager.renderBorders();
        trailManager.unBind();

        //---------------------------------------------------------------------------------------------

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        t = glfwGetTime();

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

//        quadProgram.updateUniform("Homography", glHomography);
        quadProgram.updateUniform("Homography", glBordersHomography2);

        // Find Draw -------------------------------------------------------------------------------------------------------------------------------
        glEnable(GL_BLEND);
        glViewport(0, 0, width, height);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBOVertices);
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_STATIC_DRAW);

        glBindVertexArray(quadVAO);
//        glBindTexture(GL_TEXTURE_2D, trailManager.getRenderTextureGLId());
        glBindTexture(GL_TEXTURE_2D, cvTexture);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        // Draw UI
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);



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
        double frameDeltaTime = newTime - t;
        fps = 1.f/ frameDeltaTime;
        //limit FPS :
//        if(frameDeltaTime < 1.f/60.f)
//        {
//            std::cout<<"sleep time = "<<((1.f/60.f) - frameDeltaTime)<<std::endl;
//            usleep(((1.f/60.f) - frameDeltaTime)*1000000);
//        }

        //std::cout<<"time = "<<std::endl;
        //std::cout<<"deltaTime = "<<fps<<std::endl;
        //std::cout<<"fps = "<<fps<<std::endl;


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
