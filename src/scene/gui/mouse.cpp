#include "mouse.hpp"

#include <cmath>

const float Mouse::ROTATION_ANGLE = 1.57079632679489661923F;

Mouse::Mouse(const int &width, const int &height) :    
    pressed(false),

    width(static_cast<float>(width)),
    height(static_cast<float>(height)),
    
    translation(0.0F),
    rotation(0.0F, 0.0F, 1.0F) {}

bool Mouse::isPressed() const {
    return pressed;
}

glm::vec2 Mouse::getResolution() const {
    return glm::vec2(width, height);
}

glm::vec2 Mouse::Mouse::getTranslationPoint() const {
    return translation;
}

glm::vec3 Mouse::getRotationPoint() const {
    return rotation;
}

void Mouse::setPressed(const bool &status) {
    pressed = status;
}

void Mouse::setResolution(const int &new_width, const int &new_height) {
    width = static_cast<float>(new_width);
    height = static_cast<float>(new_height);
}

void Mouse::setTranslationPoint(const double &x, const double &y) {
    translation = normalizeToWindow(x, y);
}

void Mouse::setRotationPoint(const double &x, const double &y) {
    rotation = projectToSphere(x, y);
}

glm::vec2 Mouse::translate(const double &x, const double &y) {    
    glm::vec2 point = normalizeToWindow(x, y);
    glm::vec2 delta = point - translation;
    translation = point;

    return delta;
}

glm::quat Mouse::rotate(const double &x, const double &y) {    
    glm::vec3 point = projectToSphere(x, y);
    glm::vec3 axis = glm::cross(rotation, point);
    glm::quat delta = glm::angleAxis(Mouse::ROTATION_ANGLE, axis);
    
    rotation = point;

    return glm::normalize(delta);
}

glm::vec2 Mouse::normalizeToWindow(const double &x, const double &y) {
    return glm::vec2((2.0F * static_cast<float>(x) - width) / width, (height - 2.0F * static_cast<float>(y)) / height);
}

glm::vec3 Mouse::projectToSphere(const double &x, const double &y) {    
    glm::vec2 normalized((2.0F * static_cast<float>(x) - width) / width, (height + 2.0F * static_cast<float>(y)) / height);
    glm::vec3 projection = glm::vec3(normalized, normalized.x * normalized.x + normalized.y * normalized.y);
    
    projection.z = (projection.z <= 0.5F ? glm::sqrt(1.0F - projection.z) : 1.0F / (2.0F * std::sqrt(projection.z)));
   
    return projection;
}