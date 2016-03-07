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

#include "glCamera.hpp"
#include <opencv2/opencv.hpp>


class Borders
{
private:
    std::vector<Vertex> _borderVertex;
    GLuint _vao;
    GLuint _vbo;
public:
    Borders(const std::vector<cv::Vec3d> corners, const glm::vec3& color);
    Borders(const std::vector<glm::vec3> corners, const glm::vec3& color);
    void draw();
    void initGL();
};


