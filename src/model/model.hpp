#ifndef __MODEL_HPP_
#define __MODEL_HPP_

#include "loader/modelloader.hpp"
#include "loader/modeldata.hpp"
#include "material.hpp"
#include "../scene/glslprogram.hpp"
#include "../glad/glad.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

class Model : private ModelData
{
private:
    bool enabled;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 dimension;
    glm::mat4 model_mat;
    glm::mat4 model_origin_mat;
    glm::mat3 normal_mat;
    Material *default_material;

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
    void load();
    void clear();
    void updateMatrices();

public:
    Model();
    Model(const std::string &path);
    bool isEnabled() const;
    bool isOpen() const;
    bool isMaterialOpen() const;
    std::string getName() const;
    std::string getPath() const;
    std::string getMaterialPath() const;
    Material *getMaterial(const std::size_t &index) const;
    Material *getDefaultMaterial() const;
    glm::mat4 getOriginMatrix() const;
    glm::mat4 getModelMatrix() const;
    glm::mat4 getNormalMatrix() const;
    glm::vec3 getPosition() const;
    glm::quat getRotation() const;
    glm::vec3 getRotationAngles() const;
    glm::vec3 getScale() const;
    glm::vec3 getMax() const;
    glm::vec3 getMin() const;
    std::size_t getNumberOfVertices() const;
    std::size_t getNumberOfElements() const;
    std::size_t getNumberOfTriangles() const;
    std::size_t getNumberOfMaterials() const;
    std::size_t getNumberOfTextures() const;
    void setEnabled(const bool &status);
    void setPath(const std::string &new_path);
    void setPosition(const glm::vec3 &new_position);
    void setRotation(const glm::vec3 &new_rotation);
    void setRotation(const glm::quat &new_rotation);
    void setScale(const glm::vec3 &new_scale);
    void reload();
    bool reloadMaterial();
    void resetGeometry();
    void draw(GLSLProgram *const program) const;
    void translate(const glm::vec3 &delta);
    void rotate(const glm::vec3 &delta);
    void rotate(const glm::quat &factor);
    void scale(const glm::vec3 &factor);
    virtual ~Model();
};

#endif