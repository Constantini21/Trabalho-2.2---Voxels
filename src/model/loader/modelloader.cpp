#include "modelloader.hpp"
#include "objloader.hpp"
#include <iostream>

const std::string ModelLoader::space = " \t\n\r\f\v";

ModelLoader::Vertex::Vertex() : position(0.0F),
                                uv_coord(0.0F),
                                normal(0.0F),
                                tangent(0.0F) {}

ModelLoader::ModelLoader(const std::string &path) : model_data(new ModelData(path)) {}

void ModelLoader::load()
{
    glGenVertexArrays(1, &model_data->vao);
    glBindVertexArray(model_data->vao);

    glGenBuffers(1, &model_data->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model_data->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ModelLoader::Vertex) * vertex_stock.size(), &vertex_stock[0], GL_STATIC_DRAW);

    glGenBuffers(1, &model_data->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model_data->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index_stock.size(), &index_stock[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelLoader::Vertex), reinterpret_cast<void *>(offsetof(ModelLoader::Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ModelLoader::Vertex), reinterpret_cast<void *>(offsetof(ModelLoader::Vertex, uv_coord)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelLoader::Vertex), reinterpret_cast<void *>(offsetof(ModelLoader::Vertex, normal)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ModelLoader::Vertex), reinterpret_cast<void *>(offsetof(ModelLoader::Vertex, tangent)));

    glBindVertexArray(GL_FALSE);

    vertex_stock.clear();
    index_stock.clear();
}

ModelLoader::~ModelLoader() {}

ModelData *ModelLoader::load(const std::string &path, const ModelLoader::Format &format)
{
    ModelLoader *loader = nullptr;

    switch (format)
    {
    case OBJ:
        loader = static_cast<ModelLoader *>(new OBJLoader(path));
        break;

    default:
        std::cerr << "error " << format << std::endl;
        return new ModelData(path);
    }

    if (loader->read())
    {
        loader->load();
    }

    ModelData *model_data = loader->model_data;
    delete loader;

    return model_data;
}

std::vector<Material *> ModelLoader::loadMaterial(const std::string &path, const ModelLoader::Format &format)
{
    ModelLoader *loader = nullptr;

    switch (format)
    {
    case OBJ:
        loader = static_cast<ModelLoader *>(new OBJLoader(path));
        break;
    default:
        std::cerr << "error " << format << std::endl;
        return std::vector<Material *>();
    }

    loader->readMaterial(path);

    std::vector<Material *> material_stock(loader->model_data->material_stock);

    loader->model_data->material_stock.clear();
    delete loader->model_data;
    delete loader;

    return material_stock;
}

void ModelLoader::rtrim(std::string &str)
{
    str.erase(str.find_last_not_of(ModelLoader::space) + 1);
}