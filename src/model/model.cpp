#include "model.hpp"
#include "../dirsep.h"
#include "loader/modelloader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Model::load()
{
    ModelData *model_data = ModelLoader::load(model_path, ModelLoader::OBJ);

    model_open = model_data->model_open;
    material_open = model_data->material_open;

    material_path = model_data->material_path;

    origin_mat = model_data->origin_mat;
    model_origin_mat = model_mat * origin_mat;
    min = model_data->min;
    max = model_data->max;

    vao = model_data->vao;
    vbo = model_data->vbo;
    ebo = model_data->ebo;

    material_stock = model_data->material_stock;
    object_stock = model_data->object_stock;

    vertices = model_data->vertices;
    elements = model_data->elements;
    triangles = model_data->triangles;
    textures = model_data->textures;

    default_material = new Material("Default");

    model_data->object_stock.clear();
    model_data->material_stock.clear();
    delete model_data;
}

void Model::clear()
{
    model_open = false;
    material_open = false;
    material_path.clear();

    origin_mat = glm::mat4(1.0F);
    min = glm::vec3(INFINITY);
    max = glm::vec3(-INFINITY);

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vao);

    vao = GL_FALSE;
    vbo = GL_FALSE;
    ebo = GL_FALSE;

    vertices = 0U;
    elements = 0U;
    triangles = 0U;
    textures = 0U;

    object_stock.clear();
    material_stock.clear();

    if (default_material != nullptr)
    {
        delete default_material;
        default_material = nullptr;
    }
}

void Model::updateMatrices()
{
    const glm::mat4 identity = glm::mat4(1.0F);
    const glm::mat4 translation_mat = glm::translate(identity, position);
    const glm::mat4 rotation_mat = glm::mat4_cast(rotation);
    const glm::mat4 scale_mat = glm::scale(identity, dimension);

    const glm::mat4 translation_rotation_mat = translation_mat * rotation_mat;
    model_mat = translation_rotation_mat * scale_mat;
    model_origin_mat = model_mat * origin_mat;
    normal_mat = glm::inverse(glm::transpose(translation_rotation_mat));
}

Model::Model() : ModelData(std::string()),

                 enabled(true),
                 position(0.0F),
                 rotation(glm::quat()),
                 dimension(1.0F),

                 model_mat(1.0F),
                 model_origin_mat(1.0F),
                 normal_mat(1.0F),

                 default_material(nullptr)
{
}

Model::Model(const std::string &path) : ModelData(path),

                                        enabled(true),
                                        position(0.0F),
                                        rotation(glm::quat()),
                                        dimension(1.0F),
                                        model_mat(1.0F),
                                        model_origin_mat(1.0F),
                                        normal_mat(1.0F),

                                        default_material(nullptr)
{

    load();
}

bool Model::isEnabled() const
{
    return enabled;
}

bool Model::isOpen() const
{
    return model_open;
}

bool Model::isMaterialOpen() const
{
    return material_open;
}

std::string Model::getName() const
{
    return model_path.substr(model_path.find_last_of(DIR_SEP) + 1U);
}

std::string Model::getPath() const
{
    return model_path;
}

std::string Model::getMaterialPath() const
{
    return material_path;
}

Material *Model::getMaterial(const std::size_t &index) const
{
    if (index >= material_stock.size())
    {
        std::cerr << "error: the index " << index << " is greater than the material stock (" << material_stock.size() << ")" << std::endl;
        return nullptr;
    }

    return material_stock[index];
}

Material *Model::getDefaultMaterial() const
{
    return default_material;
}

glm::mat4 Model::getOriginMatrix() const
{
    return origin_mat;
}

glm::mat4 Model::getModelMatrix() const
{
    return model_mat;
}

glm::mat4 Model::getNormalMatrix() const
{
    return normal_mat;
}

glm::vec3 Model::getPosition() const
{
    return position;
}

glm::quat Model::getRotation() const
{
    return rotation;
}

glm::vec3 Model::getRotationAngles() const
{
    return glm::degrees(glm::eulerAngles(rotation));
}

glm::vec3 Model::getScale() const
{
    return dimension;
}

glm::vec3 Model::getMax() const
{
    return max;
}

glm::vec3 Model::getMin() const
{
    return min;
}

std::size_t Model::getNumberOfVertices() const
{
    return vertices;
}

std::size_t Model::getNumberOfElements() const
{
    return elements;
}

std::size_t Model::getNumberOfTriangles() const
{
    return triangles;
}

std::size_t Model::getNumberOfMaterials() const
{
    return material_stock.size();
}

std::size_t Model::getNumberOfTextures() const
{
    return textures;
}

void Model::setEnabled(const bool &status)
{
    enabled = status;
}

void Model::setPath(const std::string &new_path)
{
    model_path = new_path;
    reload();
}

void Model::setPosition(const glm::vec3 &new_position)
{
    position = new_position;
    updateMatrices();
}

void Model::setRotation(const glm::vec3 &new_rotation)
{
    rotation = glm::quat(glm::radians(new_rotation));
    updateMatrices();
}

void Model::setRotation(const glm::quat &new_rotation)
{
    rotation = new_rotation;
    updateMatrices();
}

void Model::setScale(const glm::vec3 &new_scale)
{
    dimension = new_scale;
    updateMatrices();
}

void Model::reload()
{
    clear();

    if (!model_path.empty())
    {
        load();
    }
}

bool Model::reloadMaterial()
{
    if (material_path.empty())
    {
        std::cerr << "error: cannot reload the material if the path is empty" << std::endl;
        return false;
    }

    if (default_material != nullptr)
    {
        delete default_material;
    }
    default_material = new Material("Default");

    std::vector<Material *> material_data(ModelLoader::loadMaterial(material_path, ModelLoader::OBJ));

    const std::size_t materials = material_stock.size();
    if (material_data.size() != materials)
    {
        std::cerr << "error: the materials have changed, use the reload method to see the changues" << std::endl;

        for (const Material *const material : material_data)
        {
            delete material;
        }

        return false;
    }

    for (ModelData::Object *const object : object_stock)
    {

        const Material *const &old_material = object->material;

        for (std::size_t i = 0U; i < materials; i++)
        {
            Material *&material = material_stock[i];
            if (material == old_material)
            {

                delete old_material;

                material = material_data[i];
                object->material = material;
                break;
            }
        }
    }

    return true;
}

void Model::resetGeometry()
{

    position = glm::vec3(0.0F);
    rotation = glm::quat();
    dimension = glm::vec3(1.0F);

    updateMatrices();
}

void Model::draw(GLSLProgram *const program) const
{
    if (!enabled || !model_open || (program == nullptr) || (!program->isValid()))
    {
        return;
    }

    program->use();
    program->setUniform("u_model_mat", model_origin_mat);
    program->setUniform("u_normal_mat", normal_mat);

    glBindVertexArray(vao);

    for (const ModelData::Object *const object : object_stock)
    {
        object->material->bind(program);

        glDrawElements(GL_TRIANGLES, object->count, GL_UNSIGNED_INT, reinterpret_cast<void *>(static_cast<intptr_t>(object->offset)));
    }

    glBindVertexArray(GL_FALSE);
}

void Model::translate(const glm::vec3 &delta)
{
    position += delta;
    updateMatrices();
}

void Model::rotate(const glm::vec3 &delta)
{
    rotation = glm::normalize(glm::quat(glm::radians(delta)) * rotation);
    updateMatrices();
}

void Model::rotate(const glm::quat &factor)
{
    rotation = glm::normalize(factor * rotation);
    updateMatrices();
}

void Model::scale(const glm::vec3 &factor)
{
    dimension *= factor;

    if (!std::isfinite(dimension.x))
        dimension.x = 0.001F;
    if (!std::isfinite(dimension.y))
        dimension.y = 0.001F;
    if (!std::isfinite(dimension.z))
        dimension.z = 0.001F;

    updateMatrices();
}

Model::~Model()
{
    clear();
}