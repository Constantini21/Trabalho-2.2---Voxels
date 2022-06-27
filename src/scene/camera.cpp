#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>

bool Camera::boosted = false;
float Camera::speed = 0.5F;
float Camera::boosted_speed = 1.0F;
float Camera::sensibility = 15.0F;
float Camera::zoom_factor = 1.0625F;

void Camera::updateViewMatrix()
{
    view_mat = glm::lookAt(position, position + front, glm::cross(right, front));
}

void Camera::updateProjectionMatrices()
{
    const float aspect = width / height;
    const float ratio = std::atan(fov / 2.0F);
    const float distance = glm::length(position);
    const float y = ratio * distance;
    const float x = y * aspect;

    orthogonal_mat = glm::ortho(-x, x, -y, y, near, far);
    perspective_mat = glm::perspective(fov, aspect, near, far);
}

Camera::Camera(const int &width, const int &height, const bool &orthogonal) : orthogonal(orthogonal),
                                                                              width(static_cast<float>(width)),
                                                                              height(static_cast<float>(height))
{

    reset();
}

bool Camera::isOrthogonal() const
{
    return orthogonal;
}

glm::vec3 Camera::getPosition() const
{
    return position;
}

glm::vec3 Camera::getDirection() const
{
    return front;
}

glm::vec3 Camera::getRotation() const
{
    return glm::degrees(glm::vec3(std::atan2(front.z, front.x), std::asin(front.y), std::asin(up.x)));
}

glm::vec3 Camera::getUp() const
{
    return up;
}

float Camera::getFOV() const
{
    return glm::degrees(fov);
}

glm::vec2 Camera::getResolution() const
{
    return glm::vec2(width, height);
}

glm::vec2 Camera::getClipping() const
{
    return glm::vec2(near, far);
}

glm::mat4 Camera::getViewMatrix() const
{
    return view_mat;
}

glm::mat4 Camera::getOrthogonalMatrix() const
{
    return orthogonal_mat;
}

glm::mat4 Camera::getPerspectiveMatrix() const
{
    return perspective_mat;
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return orthogonal ? orthogonal_mat : perspective_mat;
}

void Camera::setOrthogonal(const bool &status)
{
    orthogonal = status;
}

void Camera::setPosition(const glm::vec3 &new_position)
{
    position = new_position;
    updateViewMatrix();
    updateProjectionMatrices();
}

void Camera::setDirection(const glm::vec3 &new_direction)
{
    front = glm::normalize(new_direction);
    right = glm::cross(front, up);
    updateViewMatrix();
}

void Camera::setRotation(const glm::vec3 &angle)
{
    glm::vec3 radian = glm::radians(glm::vec3(angle.x, glm::clamp(angle.y, -89.0F, 89.0F), angle.z));

    front.x = std::cos(radian.y) * std::cos(radian.x);
    front.y = std::sin(radian.y);
    front.z = std::cos(radian.y) * std::sin(radian.x);
    front = glm::normalize(front);

    up.x = std::sin(radian.z);
    up.y = std::cos(radian.z);
    up = glm::normalize(up);

    right = glm::normalize(glm::cross(front, up));

    updateViewMatrix();
}

void Camera::setUp(const glm::vec3 &direction)
{
    up = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, up));
    updateViewMatrix();
}

void Camera::setFOV(const float &new_fov)
{
    fov = glm::radians(new_fov);
    updateProjectionMatrices();
}

void Camera::setResolution(const glm::vec2 &resolution)
{
    width = resolution.x;
    height = resolution.y;

    if (height == 0.0F)
    {
        height = 1.0F;
    }

    updateProjectionMatrices();
}

void Camera::setClipping(const glm::vec2 &clipping)
{
    near = clipping.x;
    far = clipping.y;
    updateProjectionMatrices();
}

void Camera::reset()
{
    position = glm::vec3(0.0F, 0.0F, 2.0F);
    front = glm::vec3(0.0F, 0.0F, -1.0F);
    right = glm::vec3(1.0F, 0.0F, 0.0F);
    up = glm::vec3(0.0F, 1.0F, 0.0F);

    fov = glm::radians(30.0F);
    near = 0.01F;
    far = 10.0F;

    yaw = -90.0F;
    pitch = 0.0F;

    updateViewMatrix();
    updateProjectionMatrices();
}

void Camera::bind(GLSLProgram *const program) const
{
    if ((program == nullptr) || (!program->isValid()))
    {
        return;
    }

    program->use();
    program->setUniform("u_up_dir", up);
    program->setUniform("u_view_dir", front);
    program->setUniform("u_view_pos", position);
    program->setUniform("u_view_mat", view_mat);
    program->setUniform("u_projection_mat", orthogonal ? orthogonal_mat : perspective_mat);
}

void Camera::travell(const Camera::Movement &direction, const double &time)
{
    float distance = (Camera::boosted ? Camera::boosted_speed : Camera::speed) * (float)time;

    switch (direction)
    {
    case RIGHT:
        position += right * distance;
        break;
    case LEFT:
        position -= right * distance;
        break;
    case UP:
        position += up * distance;
        break;
    case DOWN:
        position -= up * distance;
        break;
    case FRONT:
        position += glm::normalize(glm::cross(up, right)) * distance;
        break;
    case BACK:
        position -= glm::normalize(glm::cross(up, right)) * distance;
        break;
    }

    updateViewMatrix();
    updateProjectionMatrices();
}

void Camera::translate(const glm::vec3 &delta)
{
    position += delta;
    updateViewMatrix();
}

void Camera::zoom(const double &direction)
{
    fov = direction > 0 ? fov / Camera::zoom_factor : fov * Camera::zoom_factor;
    updateProjectionMatrices();
}

void Camera::rotate(const glm::vec2 &delta)
{
    yaw += delta.x * Camera::sensibility;
    pitch = glm::clamp(pitch + delta.y * Camera::sensibility, -89.0F, 89.0F);

    front.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
    front.y = std::sin(glm::radians(pitch));
    front.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, up));

    updateViewMatrix();
}

Camera::~Camera() {}

bool Camera::isBoosted()
{
    return Camera::boosted;
}

float Camera::getSpeed()
{
    return Camera::speed;
}

float Camera::getBoostedSpeed()
{
    return Camera::boosted_speed;
}

float Camera::getSensibility()
{
    return Camera::sensibility;
}

float Camera::getZoomFactor()
{
    return Camera::zoom_factor;
}

void Camera::setBoosted(const bool &status)
{
    Camera::boosted = status;
}

void Camera::setSpeed(const float &speed)
{
    Camera::speed = speed;
}

void Camera::setBoostedSpeed(const float &speed)
{
    Camera::boosted_speed = speed;
}

void Camera::setSensibility(const float &sensibility)
{
    Camera::sensibility = sensibility;
}

void Camera::setZoomFactor(const float &factor)
{
    Camera::zoom_factor = factor;
}