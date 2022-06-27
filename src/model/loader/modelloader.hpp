#ifndef __MODEL_LOADER_HPP_
#define __MODEL_LOADER_HPP_
#include "modeldata.hpp"
#include "../material.hpp"
#include "../../glad/glad.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <vector>

/** Model loader abstract class */
class ModelLoader
{
protected:
    /** Model vertex */
    struct Vertex
    {
    public:
        glm::vec3 position;
        glm::vec2 uv_coord;
        glm::vec3 normal;
        glm::vec3 tangent;
        Vertex();
    };

    ModelData *model_data;
    std::vector<glm::vec3> position_stock;
    std::vector<glm::vec2> uv_coord_stock;
    std::vector<glm::vec3> normal_stock;
    std::map<std::string, GLsizei> parsed_vertex;
    std::vector<GLsizei> index_stock;
    std::vector<Vertex> vertex_stock;

    ModelLoader(const std::string &path);
    ModelLoader() = delete;
    ModelLoader(const ModelLoader &) = delete;
    ModelLoader &operator=(const ModelLoader &) = delete;
    virtual bool read() = 0;
    virtual bool readMaterial(const std::string &path) = 0;
    void load();
    static const std::string space;

public:
    enum Format
    {
        OBJ
    };

    virtual ~ModelLoader();
    static ModelData *load(const std::string &path, const ModelLoader::Format &format);
    static std::vector<Material *> loadMaterial(const std::string &path, const ModelLoader::Format &format);
    static void rtrim(std::string &str);
};

#endif