#include "modeldata.hpp"

ModelData::Object::Object(const GLsizei &count, const GLsizei &offset, Material *const material) :
    count(count),
    offset(sizeof(GLsizei) * offset),
    material(material) {}


ModelData::ModelData(const std::string &path) :    
    model_path(path),
    model_open(false),
    material_open(false),

    origin_mat(1.0F),
    min(INFINITY),
    max(-INFINITY),

    vao(GL_FALSE),
    vbo(GL_FALSE),
    ebo(GL_FALSE),
    
    vertices(0U),
    elements(0U),
    triangles(0U),
    textures(0U) {}

ModelData::~ModelData()
{    
    for (const ModelData::Object *const object : object_stock)
    {
        delete object;
    }
    
    for (const Material *const material : material_stock)
    {
        delete material;
    }
}