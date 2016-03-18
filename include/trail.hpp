#pragma once

//glm
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/geometric.hpp>
//glew
#include <GL/glew.h>
//std
#include <iostream>
#include <vector>
#include <deque>

#include "glUtils.hpp"


//////////////////////////////////////////////////////
//////////////////////// TRAIL ///////////////////////
//////////////////////////////////////////////////////

class Trail{
private:
    int _trailBuffer; //max number of point in _trailPoints
    std::vector<glm::vec3> _trailPoints;
    std::vector<Vertex> _trailVertex;
    std::vector<int> _trailIndex;
    GLuint _vao;
    GLuint _vbo;
    GLuint _ibo;
    float _trailWidth;
    int _frontIndex;
    int _backIndex;

public:
    Trail(float trailWidth = 1, int bufferSize = 16);
    ~Trail();

    void setTrailWidth(float trailWidth);

    //used for fullscreen toggle
    void reInitGL();

    void initGL();
    void pushBack(const glm::vec3& pointPosition, const glm::vec3 &color = glm::vec3(1,1,1));
    void popFront();
    void popFront_naive();
    void synchronizeVbos();
    void draw();
    void clearGL();
    void update();

    int getIndexCount();
};

