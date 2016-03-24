//glm
#include <glm/common.hpp>
#include <glm/glm.hpp>
#pragma once

#include <glm/ext.hpp>
#include <glm/geometric.hpp>
//glew
#include <GL/glew.h>
//std
#include <iostream>
#include <vector>
#include <deque>

#include "glCamera.hpp"
#include "trail.hpp"

#include <opencv2/opencv.hpp>

#include "borders.hpp"


#include "ShaderProgram.hpp"

struct InputInfo{
    bool leftButtonDown;
    bool rightButtonDown;
    glm::vec2 pointerPosition;

    InputInfo() : leftButtonDown(false), rightButtonDown(false), pointerPosition(0,0)
    {}
};

class TrailManager{
private:
    std::map<int, Trail> _trails;
    std::map<int, glm::vec3> _trailsColors;
    Camera _camera;
    GLuint _fbo;
    GLuint _renderTexture;
    GLuint _glProgram;
    GLuint _uniform_View;
    GLuint _uniform_Projection;
    int _texWidth;
    int _texHeight;

    Borders* _borders;

public:
    TrailManager(int texWidth, int texHeight, std::vector<int> trailkeys, std::vector<glm::vec3> trailColors, int trailBufferSize = 100, float trailWidth = 5);

    // called on fullscreen toggle
    void reInit();

    Camera& getCamera();
    Trail& getTrail(int key);
    std::map<int, Trail>& trails();
    int getTrailCount() const;
    //void updateFromOpenCV(const cv::Mat& camToWorld, const std::vector<int>& markerId, const std::vector<cv::Vec<double, 3>>& currentMarkerPos);
    void updateTrailPositions(const std::vector<int>& markerIds, std::map<int, glm::vec2> &currentMarkerPos);
    void updateTrails();
    void synchronizeVBOTrails();
    void render();
    void renderToTexture();
    void updateCameraPos(const std::vector<cv::Vec3d>& corners, float cameraHeight = 10);
    void convertGlTexToCVMat(cv::Mat& cvMat);
    void convertWindowBufferToCVMat(cv::Mat& cvMat);
    GLuint getRenderTextureGLId() const;
    void bind();
    void unBind();
    void renderTrails();
    void renderBorders();
    int getTexWidth() const;
    int getTexHeight() const;

    bool updateScoresCollision();
    glm::vec3 color(int keyTrail);
};

void openglDrawCalls(void* userData);
void CallBackMouseFunc(int event, int x, int y, int flags, void* userdata);
int testDrawFollowingMouse();
int testDrawToTexture();

//////////////////////////////////////////
