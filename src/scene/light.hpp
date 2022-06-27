#ifndef __LIGHT_HPP_
#define __LIGHT_HPP_

#include "glslprogram.hpp"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Light
{
public:
    enum Type : GLint
    {
        DIRECTIONAL,
        POINT,
        SPOTLIGHT
    };

private:
    bool enabled;
    bool grabbed;
    Light::Type type;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 attenuation;
    glm::vec2 cutoff;
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    float ambient_level;
    float diffuse_level;
    float specular_level;
    float shininess;

public:
    Light(const Light::Type &type = Light::DIRECTIONAL);
    bool isEnabled() const;
    bool isGrabbed() const;
    Light::Type getType() const;
    glm::vec3 getDirection() const;
    glm::vec3 getPosition() const;
    glm::vec3 getAttenuation() const;
    glm::vec2 getCutoff() const;
    glm::vec3 getAmbientColor() const;
    glm::vec3 getDiffuseColor() const;
    glm::vec3 getSpecularColor() const;
    float getAmbientLevel() const;
    float getDiffuseLevel() const;
    float getSpecularLevel() const;
    float getShininess() const;
    void setEnabled(const bool &status);
    void setGrabbed(const bool &status);
    void setType(const Light::Type &new_type);
    void setDirection(const glm::vec3 &new_direction);
    void setPosition(const glm::vec3 &new_position);
    void setAttenuation(const glm::vec3 &new_attenuation);
    void setCutoff(const glm::vec2 &new_cutoff);
    void setAmbientColor(const glm::vec3 &color);
    void setDiffuseColor(const glm::vec3& color);
    void setSpecularColor(const glm::vec3& color);
    void setAmbientLevel(const float &value);
    void setDiffuseLevel(const float &value);
    void setSpecularLevel(const float &value);
    void setShininess(const float &value);
    void bind(GLSLProgram *const program) const;
};

#endif