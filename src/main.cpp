#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imguiRenderGL3.h"

#include <glog/logging.h>
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/aruco/charuco.hpp"
#include "trailManager.hpp"
#include "ShaderProgram.hpp"
#include "markers.hpp"
#include "timer.hpp"
#include <SDL2/SDL_mixer.h>

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;

int main(int argc, char** argv) {

    cv::VideoCapture inputVideo(-1);
    if(!inputVideo.open(-1))
        throw std::runtime_error("No camera found");

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1){
        throw std::runtime_error("Cannot init SDL_Mixer");
    }

    const std::string collisionSoundName = "../assets/collision.wav";
    const std::string curveFeverSoundName = "../assets/curveFever.mp3";
    const std::string winSoundName = "../assets/win.mp3";

    Mix_Chunk* collisionSound = Mix_LoadWAV(collisionSoundName.c_str());
    Mix_Music* winMusic = Mix_LoadMUS(winSoundName.c_str());
    Mix_Music* curveFeverSound = Mix_LoadMUS(curveFeverSoundName.c_str());

    Mix_PlayMusic(curveFeverSound, -1);

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
    std::vector<std::vector<cv::Point2f>> markersCorners;
    std::vector<int> markersIds;

    cv::Mat image;

    image = cv::imread("../assets/image.png");
    getBordersScreenPositions(image, dictionary, markersCorners, markersIds, true);


    if(image.empty()){
        while(inputVideo.grab()){
            cv::Mat imageCopy;
            inputVideo.retrieve(image);
            image.copyTo(imageCopy);

            getBordersScreenPositions(imageCopy, dictionary, markersCorners, markersIds, true);

            cv::imshow("out", imageCopy);

            char key = (char) cv::waitKey(1);
            if (key == 27 && markersIds.size() == 4){
                break;
            }
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

    for(size_t i = 0; i < markersIds.size(); ++i){
        bordersPositionsOrdered[markersIds[i]] = bordersPositions[i];
        bordersUvsOrdered[markersIds[i]] = bordersUvs[i];
    }

    bordersPositions = bordersPositionsOrdered;
    bordersUvs = bordersUvsOrdered;

    DLOG(INFO) << "-------------------------------------------------";
    DLOG(INFO) << "ORDERED : ";
    for(size_t i = 0; i < markersIds.size(); ++i){
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

    glm::mat3 glScreenToViewport;
    cv::Mat cvScreenToViewport;

    glm::mat3 glScreenNormToUV;
    cv::Mat cvScreenNormToUV;

    cvScreenToViewport = cv::findHomography(bordersPositions, bordersTargetPositions);
    glScreenToViewport = convertCVMatrix3x3(cvScreenToViewport);

    cvScreenNormToUV = cv::findHomography(bordersUvs, bordersTargetUvs);
    glScreenNormToUV = convertCVMatrix3x3(cvScreenNormToUV);

//    DLOG(INFO) << "HOMOGRAPHY_UV";
//    for(auto& point : bordersUvs){
//        glm::vec3 toto(glScreenNormToUV * glm::vec3(point.x, point.y, 1));
//        toto/=toto.z;
//        DLOG(INFO) << point << " TO " << glm::to_string(toto);
//    }
//
//    DLOG(INFO) << "HOMOGRAPHY_UV_INV";
//    for(auto& point : bordersTargetUvs){
//        glm::vec3 toto(glm::inverse(glScreenNormToUV) * glm::vec3(point.x, point.y, 1));
//        toto/=toto.z;
//        DLOG(INFO) << point << " TO " << glm::to_string(toto);
//    }
//
//    DLOG(INFO) << "HOMOGRAPHY_SCREEN";
//    for(auto& point : bordersPositions){
//        glm::vec3 toto(glScreenToViewport * glm::vec3(point.x, point.y, 1));
//        toto/=toto.z;
//        DLOG(INFO) << point << " TO " << glm::to_string(toto);
//    }
//
//    DLOG(INFO) << "HOMOGRAPHY_SCREEN_INV";
//    for(auto& point : bordersTargetPositions){
//        glm::vec3 toto(glm::inverse(glScreenToViewport) * glm::vec3(point.x, point.y, 1));
//        toto/=toto.z;
//        DLOG(INFO) << point << " TO " << glm::to_string(toto);
//    }

    //set window size :
    int width = 1280, height= 720;
    //set corners position :
    std::vector<cv::Vec3d> corners = {cv::Vec3d(1,1,0), cv::Vec3d(-1,1,0), cv::Vec3d(-1,-1,0), cv::Vec3d(1,-1,0)};//{cv::Vec3d(width,height,0), cv::Vec3d(0,height,0), cv::Vec3d(0,0,0), cv::Vec3d(width,0,0)};


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

    // Simple shader that apply homography on webcam texture
    Graphics::ShaderProgram flatifyProgram("../shaders/blit.vert", "", "../shaders/flatify.frag");

    // Simple shader that stretch images for mapping
    Graphics::ShaderProgram stretchifyProgram(flatifyProgram.vShader(), "../shaders/stretchify.frag");

    //simply add borders to make mapping easier
    Graphics::ShaderProgram borderifyProgram(flatifyProgram.vShader(), "../shaders/borderify.frag");
    Graphics::ShaderProgram glitchProgram(flatifyProgram.vShader(), "../shaders/glitch.frag");
    Graphics::ShaderProgram textureProgram(flatifyProgram.vShader(), "../shaders/texture.frag");

    flatifyProgram.updateUniform("Texture", 0);
    flatifyProgram.updateUniform("Homography", glScreenNormToUV);

    stretchifyProgram.updateUniform("Texture", 0);

    // ---------------- Create quad that will contain render

    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3};
    int   quad_lineList[] = {0, 1, 3, 2};

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

    // ---------------- Create vao to flatify original image;

    GLuint flatifyVAO, flatifyVBOVertices, flatifyVBOIndexes, flatifyVBOTexcoord;

    glGenVertexArrays(1, &flatifyVAO);
    glGenBuffers(1, &flatifyVBOVertices);
    glGenBuffers(1, &flatifyVBOIndexes);
    glGenBuffers(1, &flatifyVBOTexcoord);

    glBindVertexArray(flatifyVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flatifyVBOIndexes);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOVertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, quadTexcoord.size() * sizeof(glm::vec2), quadTexcoord.data(), GL_STATIC_DRAW);

    // ---------------- Create vao to stretchify original image;

    GLuint stretchifyVAO, stretchifyVBOVertices;

    glGenVertexArrays(1, &stretchifyVAO);
    glGenBuffers(1, &stretchifyVBOVertices);

    glBindVertexArray(stretchifyVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flatifyVBOIndexes);

    glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

    // ---------------- Create vao to borderify original image;

    GLuint borderifyVAO, borderifyVBOIndexes;

    glGenVertexArrays(1, &borderifyVAO);
    glGenBuffers(1, &borderifyVBOIndexes);

    glBindVertexArray(borderifyVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, borderifyVBOIndexes);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_lineList), quad_lineList, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Launch capture -------------------------------------------------------------------------------------------------------------------------------

    cv::Mat captureImage;
    cv::Mat scoresImage;

    // Create Textures -------------------------------------------------------------------------------------------------------------------------------

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

    cv::Mat glitchTexture = cv::imread("../assets/glitch.jpg");
    GLuint glitchTextureId;
    glGenTextures(1, &glitchTextureId);
    glBindTexture(GL_TEXTURE_2D, glitchTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, glitchTexture.cols, glitchTexture.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, glitchTexture.data);

    GLuint cvScoresTexture;
    glGenTextures(1, &cvScoresTexture);
    glBindTexture(GL_TEXTURE_2D, cvScoresTexture);
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
    bool keypressedFullscreen = false;

    bool drawBackground = true;
    bool drawBorders = true;
    bool drawImgui = true;
    bool keypressedDrawImgui = false;

    bool player1Win = false;
    bool player2Win = false;

    // Create TrailManager --------------------------------------------------------------------------------------------------------------------

    int finalImgWidth = 1920;
    int finalImgHeight = 1080;

    // TODO change
    std::vector<int> trailsMarkerIds = {0,1};
    TrailManager trailManager(finalImgWidth, finalImgHeight, trailsMarkerIds, {glm::vec3(1,0,0), glm::vec3(0,1,0)}, 128, 0.1);
    trailManager.updateCameraPos(corners, 10);
    Timer trailTimer;

    // Set up Aruco -------------------------------------------------------------------------------------------------------------
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(7, 5, 1, 0.5, dictionary);

    do
    {
        // GLFW events ---------------------------------------------------------------------------------------------------------
        unsigned char mbut = 0;
        int mscroll = 0;
        double mousex; double mousey;

        glfwGetCursorPos(window, &mousex, &mousey);
        glfwGetWindowSize(window, &width, &height);

        mousex*=DPI;
        mousey*=DPI;
        mousey = height - mousey;

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
            mbut |= IMGUI_MBUT_LEFT;
        }

        t = glfwGetTime();

        // get camera image ---------------------------------------------------------------------------
        inputVideo >> captureImage;

        //Get marker positions ------------------------------------------------------------------------------------
        std::vector<int> markerIds;
        std::vector< std::vector<cv::Point2f> > markerCorners;
        cv::aruco::detectMarkers(captureImage, board->dictionary, markerCorners, markerIds);

        //markerCenters will contains the center of each marker.
        std::map<int, glm::vec2> markerCenters;
        for(size_t i = 0; i < markerCorners.size(); i++)
        {
            //get center of each corner :
            glm::vec2 sum(0,0);
            for(size_t j = 0; j < markerCorners[i].size(); j++)
            {
                sum += glm::vec2(markerCorners[i][j].x, markerCorners[i][j].y);
            }
            sum /= markerCorners[i].size();

            glm::vec3 screenPos = glScreenToViewport * glm::vec3(sum, 1);
            screenPos /= screenPos.z;

            markerCenters[markerIds[i]] = glm::vec2(screenPos);
        }

        // Draw in cvTexture ---------------------------------------------------------------------------------------

        if(markerIds.size() > 0) {
            cv::aruco::drawDetectedMarkers(captureImage, markerCorners, markerIds);
        }

        glBindTexture(GL_TEXTURE_2D, cvTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, capWidth, capHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, captureImage.data);

        //update trails ------------------------------------------------------------------------------------------------------

        //add point to trail base on the first marker position :
        bool collisionOccured = false;
        if(trailTimer.elapsedTime() > 0.003f )
        {
            trailManager.updateTrailPositions(markerIds, markerCenters);
            glBindVertexArray(flatifyVAO);
            collisionOccured = trailManager.updateScoresCollision();
            glBindVertexArray(0);
            trailTimer.restart();
        }

        // -------------------------------------------------------------------------------------------

        //update trails ------------------------------------------------------------------------------

        trailManager.updateTrails();
        trailManager.synchronizeVBOTrails();

        captureImage.copyTo(scoresImage);
        scoresImage = cv::Mat::zeros(captureImage.size(), captureImage.type());
        int yOffset = 100;
        int stepOffset = 40;

        for(auto& t: trailManager.trails()){
            glm::vec3 color = trailManager.color(t.first) * 255;
            DLOG(INFO) << color.x << " " << color.y << " " << color.z;
            int score = t.second.score()/2;
            float scoreUI = score;
            int xOffset = 0;
            int rectWidth = 20;
            for(int i =0; i<score; i+=10){
                cv::rectangle(scoresImage, cv::Point(xOffset +10, yOffset-10), cv::Point(xOffset+10+rectWidth, yOffset - stepOffset), cv::Scalar(color.x, color.y, color.z), CV_FILLED);
                xOffset += 10 + rectWidth;
            }
            yOffset += stepOffset;
        }
        glBindTexture(GL_TEXTURE_2D, cvScoresTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scoresImage.cols, scoresImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, scoresImage.data);
        glBindTexture(GL_TEXTURE_2D, 0);


        // Find Draw -------------------------------------------------------------------------------------------------------------------------------

        glClear(GL_COLOR_BUFFER_BIT);

        trailManager.bind();

        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, trailManager.getTexWidth(), trailManager.getTexHeight());

        if(drawBackground){
            flatifyProgram.useProgram();
            glBindVertexArray(flatifyVAO);
            glBindTexture(GL_TEXTURE_2D, cvTexture);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        }

        trailManager.renderTrails();

//        collisionOccured = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

        if(collisionOccured){
            glitchProgram.useProgram();
            glitchProgram.updateUniform("Texture", 0);
            glitchProgram.updateUniform("GlitchTexture", 1);
            glitchProgram.updateUniform("Random", glm::sphericalRand(1.f));
            glitchProgram.updateUniform("ScreenDim", glm::vec2(finalImgWidth, finalImgHeight));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, trailManager.getRenderTextureGLId());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, glitchTextureId);

            glBindVertexArray(flatifyVAO);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

            if(Mix_Playing(-1) == 0)
                Mix_PlayChannel(-1, collisionSound, 0);
        }

        textureProgram.useProgram();
        textureProgram.updateUniform("Texture", 0);
        glBindVertexArray(flatifyVAO);
        glBindTexture(GL_TEXTURE_2D, cvScoresTexture);
        glEnable(GL_BLEND);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        glDisable(GL_BLEND);

        trailManager.unBind();

        // Find Homography -------------------------------------------------------------------------------------------------------------------------------

        std::vector<cv::Point2f> stretchedPoints;
        for(auto& vert : quadVertices){
            stretchedPoints.push_back(cv::Point2f(vert.x, vert.y));
        }

        cv::Mat cvStretchHomography = cv::findHomography(original, stretchedPoints);
        glm::mat3 glStretchHomography = convertCVMatrix3x3(cvStretchHomography);

        stretchifyProgram.updateUniform("Homography", glStretchHomography);

        glViewport(0, 0, width, height);

        stretchifyProgram.useProgram();

        glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(stretchifyVAO);
        glBindTexture(GL_TEXTURE_2D, trailManager.getRenderTextureGLId());
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        if(drawBorders){
            glLineWidth(10);

            borderifyProgram.useProgram();

            if(collisionOccured){
                borderifyProgram.updateUniform("LineColor", glm::sphericalRand(1.f));
            }
            else{
                borderifyProgram.updateUniform("LineColor", glm::vec3(1,1,1));
            }

            glBindVertexArray(borderifyVAO);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)0);
        }

        // Draw UI
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        imguiBeginFrame(mousex, mousey, mbut, mscroll);
        char lineBuffer[512];

        float xwidth = drawImgui ? 300 : 0;
        float ywidth = drawImgui ? 500 : 0;

        float padding = drawImgui ? 10 : -10;

        imguiBeginScrollArea("car-track", width - xwidth - padding, height - ywidth - padding, xwidth, ywidth, &logScroll);

        if(drawImgui){
            sprintf(lineBuffer, "FPS %f", fps);
            imguiLabel(lineBuffer);

            if(imguiCheck("draw background", drawBackground))
                drawBackground = !drawBackground;
            if(imguiCheck("draw borders", drawBorders))
                drawBorders = !drawBorders;

            for(auto& vec : quadVertices){
                imguiSeparatorLine();
                imguiSlider("x", &vec.x, -1, 1, 0.01);
                imguiSlider("y", &vec.y, -1, 1, 0.01);
            }

            imguiEndScrollArea();
            imguiEndFrame();
        }

        imguiRenderGLDraw(width, height);

        glDisable(GL_BLEND);

        glfwSwapBuffers(window);

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keypressedFullscreen){
            fullscreen = !fullscreen;
            keypressedFullscreen = true;
            DLOG(INFO) << "SWITCH TO " << (fullscreen ? "FULLSCREEN" : "WINDOWED");

            int count;
            GLFWmonitor** monitors = glfwGetMonitors(&count);

            GLFWwindow * tmpWindow = glfwCreateWindow(width, height, "car-track", fullscreen ? monitors[count-1] : NULL, window);
            glfwMakeContextCurrent(tmpWindow);
            glfwDestroyWindow(window);
            window = tmpWindow;
            GLenum err = glewInit();
            glfwSwapBuffers(window);

            glDeleteVertexArrays(1, &flatifyVAO);
            glGenVertexArrays(1, &flatifyVAO);

            glBindVertexArray(flatifyVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flatifyVBOIndexes);

            glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOVertices);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            // ---------------- Create vao to stretchify original image;

            glDeleteVertexArrays(1, &stretchifyVAO);
            glGenVertexArrays(1, &stretchifyVAO);

            glBindVertexArray(stretchifyVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flatifyVBOIndexes);

            glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            // ---------------- Create vao to borderify original image;

            glDeleteVertexArrays(1, &borderifyVAO);
            glGenVertexArrays(1, &borderifyVAO);

            glBindVertexArray(borderifyVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, borderifyVBOIndexes);

            glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, flatifyVBOTexcoord);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            trailManager.reInit();

            if (!imguiRenderGLInit(DroidSans_ttf, DroidSans_ttf_len))
            {
                fprintf(stderr, "Could not init GUI renderer.\n");
                exit(EXIT_FAILURE);
            }

            glfwSwapInterval(1);

            glClear(GL_COLOR_BUFFER_BIT);
        }

        glfwPollEvents();

        double newTime = glfwGetTime();
        double frameDeltaTime = newTime - t;
        fps = 1.f/ frameDeltaTime;

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && keypressedFullscreen){
            keypressedFullscreen = false;
        }

        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !keypressedDrawImgui){
            drawImgui = !drawImgui;
            keypressedDrawImgui = true;
            DLOG(INFO) << "DRAW IMGUI " << drawImgui;
        }

        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && keypressedDrawImgui){
            keypressedDrawImgui = false;
        }

    } while((glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose(window)) && !player1Win && !player2Win);

    Mix_PlayMusic(winMusic, 1);

    std::string winPath = "../assets/";
    winPath += (player1Win ? "win1.jpg" : "win2.jpg");
    DLOG(INFO) << "winPath" << winPath;

    cv::Mat winTexture = cv::imread(winPath);
    cv::flip(winTexture, winTexture, -1);

    GLuint winTextureId;
    glGenTextures(1, &winTextureId);
    glBindTexture(GL_TEXTURE_2D, winTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winTexture.cols, winTexture.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, winTexture.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    do{

        glClear(GL_COLOR_BUFFER_BIT);

        trailManager.bind();

        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, trailManager.getTexWidth(), trailManager.getTexHeight());

        textureProgram.useProgram();
        textureProgram.updateUniform("Texture", 0);
        glBindVertexArray(flatifyVAO);
        glBindTexture(GL_TEXTURE_2D, winTextureId);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        trailManager.unBind();

        glViewport(0, 0, width, height);

        stretchifyProgram.useProgram();

        glBindBuffer(GL_ARRAY_BUFFER, stretchifyVBOVertices);
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(glm::vec2), quadVertices.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(stretchifyVAO);
        glBindTexture(GL_TEXTURE_2D, trailManager.getRenderTextureGLId());
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

//        if(drawBorders){
//            glLineWidth(10);
//
//            borderifyProgram.useProgram();
//
//            if(collisionOccured){
//                borderifyProgram.updateUniform("LineColor", glm::sphericalRand(1.f));
//            }
//            else{
//                borderifyProgram.updateUniform("LineColor", glm::vec3(1,1,1));
//            }
//
//            glBindVertexArray(borderifyVAO);
//            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)0);
//        }

        glfwSwapBuffers(window);

        glfwSwapInterval(1);

        glfwPollEvents();

    } while(glfwGetKey( window, GLFW_KEY_Q ) != GLFW_PRESS && !glfwWindowShouldClose(window));

    glfwDestroyWindow(window);

    DLOG(INFO) << "END PROGRAM";

    Mix_FreeChunk(collisionSound);
    Mix_FreeMusic(curveFeverSound);
    Mix_CloseAudio();
    glfwTerminate();

    return 0;
}
