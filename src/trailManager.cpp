#include "trailManager.hpp"

static const std::string  fragmentShader = "#version 330 core\n \
        in vec3 vFragColor;\n \
        in vec2 vUVCoord;\n \
        out vec4 fFragColor;\n \
        void main() {\n \
            fFragColor = vec4(1,0,0/*vFragColor*/, 1);\n \
        }";

static const std::string vertexShader= "#version 330 core\n \
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



TrailManager::TrailManager(int texWidth, int texHeight, int trailCount, int trailBufferSize, float trailWidth):
    _camera(Camera::CameraType::ORTHOGRAPHIC)
{
    _texWidth = texWidth;
    _texHeight = texHeight;

    //default values for projection :
    _camera.setOrthographicProjection(0, texWidth, 0, texHeight);

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
    for(int i = 0; i < trailCount; i++)
    {
        _trails.push_back(Trail(trailWidth, trailBufferSize));
        _trails[i].initGL();
    }
}

Camera& TrailManager::getCamera()
{
    return _camera;
}

void TrailManager::updateFromOpenCV(const cv::Mat& camToWorld, const std::vector<int>& markerId, const std::vector<cv::Vec<double, 3>>& currentMarkerPos)
{
    for(int i = 0; i < std::min(markerId.size(), _trails.size()); i++)
    {
        cv::Vec3d tmp;
        cv::transform(currentMarkerPos, tmp, camToWorld);

        //add a point a the trail :
        _trails[i].pushBack(glm::vec3(tmp[0], tmp[2], tmp[1]));
        _trails[i].update(); //automaticaly erase points at the end of trails.
    }
}

void TrailManager::renderToTexture()
{
    glViewport(0, 0, _texWidth, _texHeight);
    //draw
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_glProgram);
        glUniformMatrix4fv( _uniform_Projection, 1, false, glm::value_ptr(_camera.getProjectionMat()));
        glUniformMatrix4fv( _uniform_View, 1, false, glm::value_ptr(_camera.getViewMat()));

    for(int i = 0; i < _trails.size(); i++)
        _trails[i].draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TrailManager::updateCameraPos(const std::vector<cv::Vec3d>& corners, float cameraHeight)
{
    glm::vec3 sum(0,0,0);
    for(int i = 0; i < corners.size(); i++){
        glm::vec3 t(corners[i][0], corners[i][2], corners[i][1]);
        sum += t;
    }
    sum /= corners.size();

    _camera.setPosition(glm::vec3(sum.x, cameraHeight, sum.z));
    _camera.setOrthographicProjection(-WINDOW_WIDTH*0.5f, WINDOW_WIDTH*0.5f, -WINDOW_HEIGHT*0.5f, WINDOW_HEIGHT*0.5f);
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
