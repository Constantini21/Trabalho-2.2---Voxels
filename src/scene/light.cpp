#include "light.hpp"

#include <glm/trigonometric.hpp>

Light::Light(const Light::Type &type) :

                                        enabled(true),
                                        grabbed(false),

                                        type(type),

                                        direction(0.0F, 0.0F, 1.0F),
                                        position(0.0F, 0.0F, 2.0F),

                                        attenuation(1.0F, 0.14F, 0.007F),
                                        cutoff(glm::radians(5.0F), glm::radians(7.5F)),

                                        ambient_color(1.0F),
                                        diffuse_color(1.0F),
                                        specular_color(1.0F),

                                        ambient_level(1.0F),
                                        diffuse_level(1.0F),
                                        specular_level(1.0F),
                                        shininess(1.0F)
{
}

bool Light::isEnabled() const
{
    return enabled;
}

bool Light::isGrabbed() const
{
    return grabbed;
}

Light::Type Light::getType() const
{
    return type;
}

glm::vec3 Light::getDirection() const
{
    return -direction;
}

glm::vec3 Light::getPosition() const
{
    return position;
}

glm::vec3 Light::getAttenuation() const
{
    return attenuation;
}

glm::vec2 Light::getCutoff() const
{
    return glm::degrees(cutoff);
}

glm::vec3 Light::getAmbientColor() const
{
    return ambient_color;
}

glm::vec3 Light::getDiffuseColor() const
{
    return diffuse_color;
}

glm::vec3 Light::getSpecularColor() const
{
    return specular_color;
}

float Light::getAmbientLevel() const
{
    return ambient_level;
}

float Light::getDiffuseLevel() const
{
    return diffuse_level;
}

float Light::getSpecularLevel() const
{
    return specular_level;
}

float Light::getShininess() const
{
    return shininess;
}

void Light::setEnabled(const bool &status)
{
    enabled = status;
}

void Light::setGrabbed(const bool &status)
{
    grabbed = status;
}

void Light::setType(const Light::Type &new_type)
{
    type = new_type;
}

void Light::setDirection(const glm::vec3 &new_direction)
{
    direction = -glm::normalize(new_direction);
}

void Light::setPosition(const glm::vec3 &new_position)
{
    position = new_position;
}

void Light::setAttenuation(const glm::vec3 &new_attenuation)
{
    attenuation = new_attenuation;
}

void Light::setCutoff(const glm::vec2 &new_cutoff)
{
    cutoff = glm::radians(new_cutoff);
}

void Light::setAmbientColor(const glm::vec3 &color)
{
    ambient_color = color;
}

void Light::setDiffuseColor(const glm::vec3 &color)
{
    diffuse_color = color;
}

void Light::setSpecularColor(const glm::vec3 &color)
{
    specular_color = color;
}

void Light::setAmbientLevel(const float &value)
{
    ambient_level = value;
}

void Light::setDiffuseLevel(const float &value)
{
    diffuse_level = value;
}

void Light::setSpecularLevel(const float &value)
{
    specular_level = value;
}

void Light::setShininess(const float &value)
{
    shininess = value;
}

void Light::bind(GLSLProgram *const program) const
{

    if ((program == nullptr) || (!program->isValid()))
    {
        return;
    }

    program->use();

    if (enabled)
    {
        program->setUniform("u_light_type", type);

        if (type != Light::POINT)
        {
            program->setUniform("u_light_direction", direction);
        }

        if (type != Light::DIRECTIONAL)
        {
            program->setUniform("u_light_position", position);
            program->setUniform("u_light_attenuation", attenuation);
            if (type == Light::SPOTLIGHT)
            {
                program->setUniform("u_light_cutoff", glm::cos(cutoff));
            }
        }

        program->setUniform("u_ambient", ambient_level * ambient_color);
        program->setUniform("u_diffuse", diffuse_level * diffuse_color);
        program->setUniform("u_specular", specular_level * specular_color);
        program->setUniform("u_shininess", shininess);
    }

    else
    {
        program->setUniform("u_light_type", Light::DIRECTIONAL);

        program->setUniform("u_ambient", glm::vec3(0.0F));
        program->setUniform("u_diffuse", glm::vec3(0.0F));
        program->setUniform("u_specular", glm::vec3(0.0F));
    }
}