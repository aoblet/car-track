#include <time.h>
#include <unistd.h>
#include "trailManager.hpp"
#include <glm/gtc/random.hpp>

static const std::string  fragmentShader = "#version 410 core\n \
        in vec3 vFragColor;\n \
        in vec2 vUVCoord;\n \
        out vec4 fFragColor;\n \
        void main() {\n \
            fFragColor = vec4(vFragColor, 1);\n \
        }";

static const std::string vertexShader= "#version 410 core\n \
        layout(location = 0) in vec3 VertexPosition;\n \
        layout(location = 1) in vec3 VertexColor;\n \
        layout(location = 2) in vec2 VertexUVCoord;\n \
        out vec3 vFragColor;\n \
        out vec2 vUVCoord;\n \
        uniform mat4 ProjectionMatrix;\n \
        uniform mat4 ViewMatrix;\n \
        void main() {\n \
            vFragColor = VertexColor;\n \
            vUVCoord = VertexUVCoord;\n \
            gl_Position = ProjectionMatrix * ViewMatrix * vec4(VertexPosition, 1);\n \
        }";



TrailManager::TrailManager(int texWidth, int texHeight, std::vector<int> trailkeys, std::vector<glm::vec3> trailColors, int trailBufferSize, float trailWidth):
    _camera(Camera::CameraType::ORTHOGRAPHIC)
{
    _texWidth = texWidth;
    _texHeight = texHeight;

    //default values for projection :
    _camera.setOrthographicProjection(0, texWidth, texHeight, 0);

    // opengl initialization :
    _glProgram = createGlProgram(vertexShader, fragmentShader);
    _uniform_Projection = glGetUniformLocation(_glProgram, "ProjectionMatrix");
    _uniform_View = glGetUniformLocation(_glProgram, "ViewMatrix");


    glGenTextures(1, &_renderTexture);
    glBindTexture(GL_TEXTURE_2D, _renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _texWidth, _texHeight, 0, GL_RGB, GL_FLOAT, (void*)0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    GLuint color_attachment[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, color_attachment);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderTexture, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr<<"error on building framebuffer."<<std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // trail creation :
    if(trailkeys.size() > trailColors.size())
    {
        for(size_t i = trailColors.size(); i < trailkeys.size(); i++)
        {
            trailColors.push_back(glm::vec3(1,1,1));
        }
    }
    for(size_t i = 0; i < trailkeys.size(); i++)
    {
        _trails[trailkeys[i]] = Trail(trailWidth, trailBufferSize);
        _trails[trailkeys[i]].initGL();

        _trailsColors[trailkeys[i]] = trailColors[i];
    }
}


void TrailManager::reInit() {

    glDeleteFramebuffers(1, &_fbo);
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    GLuint color_attachment[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, color_attachment);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderTexture, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr<<"error on building framebuffer."<<std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for(auto& trail : _trails){
        trail.second.reInitGL();
    }

//    std::for_each(_trails.begin(), _trails.end(), [](auto trail){trail->reInitGL();});
}

Camera& TrailManager::getCamera()
{
    return _camera;
}

Trail &TrailManager::getTrail(int key)
{
    assert(_trails.find(key) != _trails.end());

    return _trails[key];
}

int TrailManager::getTrailCount() const
{
    return _trails.size();
}


//void TrailManager::updateFromOpenCV(const cv::Mat& camToWorld, const std::vector<int>& markerId, const std::vector<cv::Vec<double, 3>>& currentMarkerPos)
//{
//    for(int i = 0; i < std::min(markerId.size(), _trails.size()); i++)
//    {
//        cv::Vec3d tmp;
//        cv::transform(currentMarkerPos, tmp, camToWorld);

//        //add a point a the trail :
//        _trails[i].pushBack(glm::vec3(tmp[0], tmp[2], tmp[1]));
//        _trails[i].update(); //automaticaly erase points at the end of trails.
//    }
//}

void TrailManager::updateTrailPositions(const std::vector<int>& markerIds, std::map<int, glm::vec2> &currentMarkerPos)
{
    for(size_t i = 0; i < markerIds.size(); i++)
    {
        if(_trails.find(markerIds[i]) != _trails.end())
        {
            _trails[markerIds[i]].pushBack(glm::vec3(currentMarkerPos[markerIds[i]].x, currentMarkerPos[markerIds[i]].y, 0), _trailsColors[markerIds[i]]);
        }
    }
}

void TrailManager::updateTrails()
{
    for(size_t i = 0; i < _trails.size(); i++)
        _trails[i].update();
}

void TrailManager::synchronizeVBOTrails()
{
    for(size_t i = 0; i < _trails.size(); i++)
        _trails[i].synchronizeVbos();
}


void TrailManager::render()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_glProgram);
        glUniformMatrix4fv( _uniform_Projection , 1, false, glm::value_ptr(_camera.getProjectionMat()));
        glUniformMatrix4fv( _uniform_View , 1, false, glm::value_ptr(_camera.getViewMat()));

    for(size_t i = 0; i < _trails.size(); i++)
        _trails[i].draw();

    if(_borders != nullptr)
        _borders->draw();
}

void TrailManager::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
}


void TrailManager::unBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TrailManager::renderTrails()
{
    //glViewport(0, 0, _texWidth, _texHeight);

    glUseProgram(_glProgram);
        glUniformMatrix4fv( _uniform_Projection , 1, false, glm::value_ptr(_camera.getProjectionMat()));
        glUniformMatrix4fv( _uniform_View , 1, false, glm::value_ptr(_camera.getViewMat()));

    for(size_t i = 0; i < _trails.size(); i++)
        _trails[i].draw();
}

void TrailManager::renderBorders()
{
    glViewport(0, 0, _texWidth, _texHeight);

    glUseProgram(_glProgram);
        glUniformMatrix4fv( _uniform_Projection , 1, false, glm::value_ptr(_camera.getProjectionMat()));
        glUniformMatrix4fv( _uniform_View , 1, false, glm::value_ptr(_camera.getViewMat()));


    if(_borders != nullptr)
        _borders->draw();
}

int TrailManager::getTexWidth() const
{
    return _texWidth;
}

int TrailManager::getTexHeight() const
{
    return _texHeight;
}

void TrailManager::renderToTexture()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glm::vec3 color = glm::vec3(rand()%255, rand()%255, rand()%255);
    //glClearColor(color.x/255.f,color.y/255.f,color.z/255.f,1);
    //glViewport(-_texWidth*0.5f, _texWidth*0.5f, -_texHeight*0.5f, _texHeight*0.5f);
    //draw
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_glProgram);
        glUniformMatrix4fv( _uniform_Projection , 1, false, glm::value_ptr(_camera.getProjectionMat()));
        glUniformMatrix4fv( _uniform_View , 1, false, glm::value_ptr(_camera.getViewMat()));

    for(size_t i = 0; i < _trails.size(); i++)
        _trails[i].draw();

    if(_borders != nullptr)
        _borders->draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TrailManager::updateCameraPos(const std::vector<cv::Vec3d>& corners, float cameraHeight)
{

    glm::vec3 sum(0,0,0);
    for(size_t i = 0; i < corners.size(); i++){
        glm::vec3 t(corners[i][0], corners[i][2], corners[i][1]);
        sum += t;
    }
    sum /= corners.size();

//    std::vector<glm::vec3> glmCorners;
//    for(int i = 0; i < corners.size(); i++)
//    {
//        glmCorners.push_back( glm::vec3(corners[i][0] - _texWidth*0.5f, corners[i][1], corners[i][2] + _texHeight*0.5f) );
//    }

    _borders = new Borders(corners, glm::vec3(1,1,1));

    _camera.setPosition(glm::vec3(0, 0, cameraHeight));
    //_camera.setOrthographicProjection(0, _texWidth, _texHeight, 0);//(-_texWidth*0.5f, _texWidth*0.5f, -_texHeight*0.5f, _texHeight*0.5f);
    _camera.setOrthographicProjection(-1, 1, -1, 1);
}

void TrailManager::convertGlTexToCVMat(cv::Mat& cvMat)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

        //use fast 4-byte alignment (default anyway) if possible
        glPixelStorei(GL_PACK_ALIGNMENT, (cvMat.step & 3) ? 1 : 4);
        //set length of one complete row in destination data (doesn't need to equal img.cols)
        glPixelStorei(GL_PACK_ROW_LENGTH, cvMat.step/cvMat.elemSize());
        glReadPixels(0, 0, cvMat.cols, cvMat.rows, GL_BGR, GL_UNSIGNED_BYTE, cvMat.data);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TrailManager::convertWindowBufferToCVMat(cv::Mat& cvMat)
{
    //use fast 4-byte alignment (default anyway) if possible
    glPixelStorei(GL_PACK_ALIGNMENT, (cvMat.step & 3) ? 1 : 4);
    //set length of one complete row in destination data (doesn't need to equal img.cols)
    glPixelStorei(GL_PACK_ROW_LENGTH, cvMat.step/cvMat.elemSize());
    glReadPixels(0, 0, cvMat.cols, cvMat.rows, GL_BGR, GL_UNSIGNED_BYTE, cvMat.data);
}

GLuint TrailManager::getRenderTextureGLId() const
{
    return _renderTexture;
}

void openglDrawCalls(void* userData)
{
    TrailManager* trailManager = static_cast<TrailManager*>(userData);

    //synchronise opengl buffers :
    trailManager->synchronizeVBOTrails();
    trailManager->render();
}

void CallBackMouseFunc(int event, int x, int y, int flags, void* userdata)
{
    InputInfo* inputInfo = static_cast<InputInfo*>(userdata);

    inputInfo->pointerPosition = glm::vec2(x,y);

    if ( event == cv::EVENT_LBUTTONDOWN )
    {
        inputInfo->leftButtonDown = true;
    }
    else if ( event == cv::EVENT_LBUTTONUP )
    {
        inputInfo->leftButtonDown = false;
    }
    else if (event == cv::EVENT_RBUTTONDOWN )
    {
        inputInfo->rightButtonDown = true;
    }
    else if (event == cv::EVENT_RBUTTONUP )
    {
        inputInfo->rightButtonDown = false;
    }
}

int testDrawToTexture()
{
    InputInfo inputInfo;

    std::vector<cv::Vec3d> corners = {cv::Vec3d(800,600,0), cv::Vec3d(0,600,0), cv::Vec3d(0,0,0), cv::Vec3d(800,0,0)};

    cv::namedWindow("window", cv::WINDOW_NORMAL|cv::WINDOW_OPENGL);
    cv::resizeWindow("window", 800, 600);

    cv::namedWindow("window2", cv::WINDOW_NORMAL);
    cv::resizeWindow("window2", 800, 600);

    // Initialize glew for OpenGL3+ support :
    GLenum glewInitError = glewInit();
    if(GLEW_OK != glewInitError) {
        std::cerr << glewGetErrorString(glewInitError) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version : " << glewGetString(GLEW_VERSION) << std::endl;


    //create TrailManager
    int finalImgWidth = 800; // ???
    int finalImgHeight = 600; // ???
    TrailManager trailManager(finalImgWidth, finalImgHeight, {0, 1}, {glm::vec3(1,0,0), glm::vec3(0,1,0)}, 128, 10);
    trailManager.updateCameraPos(corners, 10);
    cv::Mat trailImg(finalImgHeight, finalImgWidth, CV_8UC3);

    //test loop :
    bool leave = false;
    double t = (double)cv::getTickCount();
    //set the opengl context :
    cv::setOpenGlContext("window");
    cv::setOpenGlDrawCallback("window", openglDrawCalls, &trailManager);
    while(!leave)
    {
        t = (double)cv::getTickCount();

        //add point to trail :
        cv::setMouseCallback("window", CallBackMouseFunc, &inputInfo);
        if ( inputInfo.leftButtonDown )
        {
            std::cout<<"add to trail 01"<<std::endl;
             trailManager.getTrail(0).pushBack(glm::vec3(inputInfo.pointerPosition.x, inputInfo.pointerPosition.y, 0), glm::vec3(1,0,0));
             //std::cout << "Mouse move over the window - position (" << inputInfo.pointerPosition.x << ", " << inputInfo.pointerPosition.y << ")" << std::endl;
        }
        else if (inputInfo.rightButtonDown)
        {
            std::cout<<"add to trail 02"<<std::endl;
            if(trailManager.getTrailCount() > 1)
               trailManager.getTrail(1).pushBack(glm::vec3(inputInfo.pointerPosition.x, inputInfo.pointerPosition.y, 0), glm::vec3(0,1,0));
        }

        //TODO : opencv mapping to render the trails

        //cv::ogl::Texture2D tex();
        char key = (char) cv::waitKey(1);

        //display the final image :
        cv::updateWindow("window");

        //render the trails :
        //trailManager.renderToTexture();
        //TODO : get the opengl texture and give it to openCV, modify trailImg
        //trailManager.convertGlTexToCVMat(trailImg);
        trailManager.convertWindowBufferToCVMat(trailImg);
        //the result img may be flipped :
        cv::flip(trailImg, trailImg, 0);
        //display the final image :
        cv::imshow("window2", trailImg);

        //update all trails :
        trailManager.updateTrails();

        if (key == 27)
            break;

        t = (double)(cv::getTickCount() - t)/cv::getTickFrequency();
        //std::cout << "Times passed in seconds: " << t << std::endl;
        //std::cout << "frame per second: " << 1.f / t << std::endl;
        if(t < 1.f/30.f)
        {
            usleep(((1.f/30.f) - t)*1000000);
        }
    }

}

int testDrawFollowingMouse()
{

    InputInfo inputInfo;

    cv::namedWindow("window", cv::WINDOW_NORMAL|cv::WINDOW_OPENGL);
    cv::resizeWindow("window", 800, 600);

    // Initialize glew for OpenGL3+ support :
    GLenum glewInitError = glewInit();
    if(GLEW_OK != glewInitError) {
        std::cerr << glewGetErrorString(glewInitError) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version : " << glewGetString(GLEW_VERSION) << std::endl;


    //create TrailManager
    int finalImgWidth = 800; // ???
    int finalImgHeight = 600; // ???
    TrailManager trailManager(finalImgWidth, finalImgHeight, {0, 1}, {glm::vec3(1,0,0), glm::vec3(0,1,0)}, 128, 10);

    //test loop :
    bool leave = false;
    double t = (double)cv::getTickCount();
    //set the opengl context :
    cv::setOpenGlContext("window");
    cv::setOpenGlDrawCallback("window", openglDrawCalls, &trailManager);
    while(!leave)
    {
        t = (double)cv::getTickCount();

        //add point to trail :
        cv::setMouseCallback("window", CallBackMouseFunc, &inputInfo);
        if ( inputInfo.leftButtonDown )
        {
            std::cout<<"add to trail 01"<<std::endl;
             trailManager.getTrail(0).pushBack(glm::vec3(inputInfo.pointerPosition.x, inputInfo.pointerPosition.y, 0), glm::vec3(1,0,0));
             //std::cout << "Mouse move over the window - position (" << inputInfo.pointerPosition.x << ", " << inputInfo.pointerPosition.y << ")" << std::endl;
        }
        else if (inputInfo.rightButtonDown)
        {
            std::cout<<"add to trail 02"<<std::endl;
            if(trailManager.getTrailCount() > 1)
               trailManager.getTrail(1).pushBack(glm::vec3(inputInfo.pointerPosition.x, inputInfo.pointerPosition.y, 0), glm::vec3(0,1,0));
        }

        //TODO : opencv mapping to render the trails

        //cv::ogl::Texture2D tex();
        char key = (char) cv::waitKey(1);

        //display the final image :
        cv::updateWindow("window");

        //update all trails :
        trailManager.updateTrails();

        if (key == 27)
            break;

        t = (double)(cv::getTickCount() - t)/cv::getTickFrequency();
        //std::cout << "Times passed in seconds: " << t << std::endl;
        //std::cout << "frame per second: " << 1.f / t << std::endl;
        if(t < 1.f/30.f)
        {
            usleep(((1.f/30.f) - t)*1000000);
        }
    }
}

std::map<int, Trail> &TrailManager::trails() {
    return _trails;
}

bool TrailManager::updateScoresCollision() {
    bool collide = false;
    for(auto& currentTrail: _trails){
        for(auto& otherTrail: _trails){
            if(otherTrail.first == currentTrail.first)
                continue;

            if(currentTrail.second.isCollide(otherTrail.second, 0.1)){
                collide = true;

                for(auto& scoreTrail: _trails) {
                    if(scoreTrail.first == currentTrail.first)
                        continue;
                    ++scoreTrail.second.score();
                }
            }
        }
    }
    return collide;
}

glm::vec3 TrailManager::color(int keyTrail) {
    return _trailsColors[keyTrail];
}
