#include "glCamera.hpp"


Camera::Camera(CameraType cameraType) : _position(0,0,10), _cameraType(cameraType)
{
    if(cameraType == CameraType::ORTHOGRAPHIC)
        _projectionMat = glm::ortho( 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.0f, 0.001f, 100.f);
    else
        _projectionMat = glm::perspective(60.f, WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.001f, 1000.f);

    _viewMat = glm::lookAt(_position, _position + glm::vec3(0,0,-1), glm::vec3(0,1,0));

    setPosition(_position);
}

void Camera::setOrthographicProjection(float left, float right, float bottom, float top)
{
    _projectionMat = glm::ortho( left, right, bottom, top, 0.001f, 100.f);
}

const glm::mat4& Camera::getProjectionMat() const
{
    return _projectionMat;
}

const glm::mat4& Camera::getViewMat() const
{
    return _viewMat;
}

const glm::mat4& Camera::getModelMat() const
{
    return _modelMat;
}

const glm::vec3& Camera::getPosition() const
{
    return _position;
}

void Camera::setPosition(const glm::vec3& pos)
{
    _position = pos;
    _viewMat = glm::lookAt(_position, _position + glm::vec3(0,0,-1),  glm::vec3(0,1,0) );
    _modelMat = glm::translate(glm::mat4(1), _position);
}

