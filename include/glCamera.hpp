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

//////////////////////////////////////////////
/////////////////// CAMERA ///////////////////
//////////////////////////////////////////////

class Camera{
public:
    enum CameraType {ORTHOGRAPHIC, PERSPECTIVE};

private:
    glm::vec3 _position;
    glm::mat4 _projectionMat;
    glm::mat4 _viewMat;
    glm::mat4 _modelMat;

    CameraType _cameraType;

public:
    Camera(CameraType cameraType = CameraType::ORTHOGRAPHIC);

    void setOrthographicProjection(float left, float right, float bottom, float top);

    const glm::mat4& getProjectionMat() const;
    const glm::mat4& getViewMat() const;
    const glm::mat4& getModelMat() const;

    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& pos);

};

//////////////////////////////////////////////////////


