#ifndef __MODEL_DATA_HPP_
#define __MODEL_DATA_HPP_

#include "../material.hpp"
#include "../../glad/glad.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <map>
#include <vector>

class ModelData
{
public:
    struct Object
    {
        GLsizei count;
        GLsizei offset;
        Material *material;
        Object(const GLsizei &count = 0, const GLsizei &offset = 0, Material *const material = nullptr);
    };

    std::string model_path;
    std::string material_path;
    bool model_open;
    bool material_open;
    glm::mat4 origin_mat;
    glm::vec3 min;
    glm::vec3 max;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    std::vector<ModelData::Object *> object_stock;
    std::vector<Material *> material_stock;
    std::size_t vertices;
    std::size_t elements;
    std::size_t triangles;
    std::size_t textures;

    ModelData() = delete;
    ModelData(const std::string &path);
    virtual ~ModelData();
};

#endif