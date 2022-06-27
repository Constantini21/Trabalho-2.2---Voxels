#ifndef __MOUSE_HPP_
#define __MOUSE_HPP_

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Mouse
{
private:
    bool pressed;
    float width;
    float height;
    glm::vec2 translation;
    glm::vec3 rotation;
    Mouse() = delete;
    static const float ROTATION_ANGLE;

public:
    Mouse(const int &width, const int &height);
    bool isPressed() const;
    glm::vec2 getResolution() const;
    glm::vec2 getTranslationPoint() const;
    glm::vec3 getRotationPoint() const;
    void setPressed(const bool &status);
    void setResolution(const int &width, const int &height);
    void setTranslationPoint(const double &x, const double &y);
    void setRotationPoint(const double &x, const double &y);
    glm::vec2 translate(const double &x, const double &y);
    glm::quat rotate(const double &x, const double &y);
    glm::vec2 normalizeToWindow(const double &x, const double &y);
    glm::vec3 projectToSphere(const double &x, const double &y);
};

#endif