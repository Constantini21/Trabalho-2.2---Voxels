#include "objloader.hpp"
#include "../../dirsep.h"
#include "../../glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <algorithm>
#include <vector>
#include <cctype>
#include <fstream>
#include <sstream>
#include <iostream>

GLsizei OBJLoader::storeVertex(const std::string &vertex_str)
{

    std::map<std::string, GLsizei>::iterator result = parsed_vertex.find(vertex_str);

    if (result != parsed_vertex.end())
    {
        index_stock.emplace_back(result->second);
        return result->second;
    }

    ModelLoader::Vertex vertex;

    for (std::size_t i = 0, begin = 0, end = 0; (i < 3) && (end != std::string::npos); i++, begin = end + 1)
    {

        end = vertex_str.find('/', begin);

        if ((begin < vertex_str.size()) && (begin < end))
        {
            switch (i)
            {
            case 0:
                vertex.position = position_stock[std::stoi(vertex_str.substr(begin)) - 1];
                break;
            case 1:
                vertex.uv_coord = uv_coord_stock[std::stoi(vertex_str.substr(begin)) - 1];
                break;
            case 2:
                vertex.normal = normal_stock[std::stoi(vertex_str.substr(begin)) - 1];
            }
        }
    }

    GLsizei index = static_cast<GLsizei>(vertex_stock.size());
    parsed_vertex[vertex_str] = index;
    index_stock.emplace_back(index);
    vertex_stock.emplace_back(vertex);

    return index;
}

void OBJLoader::calcTangent(const GLsizei &ind_0, const GLsizei &ind_1, const GLsizei &ind_2)
{
    ModelLoader::Vertex &vertex_0 = vertex_stock.at(ind_0);
    ModelLoader::Vertex &vertex_1 = vertex_stock.at(ind_1);
    ModelLoader::Vertex &vertex_2 = vertex_stock.at(ind_2);

    const glm::vec3 l0(vertex_1.position - vertex_0.position);
    const glm::vec3 l1(vertex_2.position - vertex_0.position);

    const glm::vec2 d0(vertex_1.uv_coord - vertex_0.uv_coord);
    const glm::vec2 d1(vertex_2.uv_coord - vertex_0.uv_coord);

    const glm::vec3 tangent((l0 * d1.t - l1 * d0.t) / glm::abs(d0.s * d1.t - d1.s * d0.t));

    vertex_0.tangent += tangent;
    vertex_1.tangent += tangent;
    vertex_2.tangent += tangent;
}

bool OBJLoader::read()
{

    std::ifstream file(model_data->model_path);
    if (!file.is_open())
    {
        std::cerr << "error " << model_data->model_path << " " << std::endl;
        return false;
    }

    std::vector<std::string> face;
    std::string token;
    std::string line;
    glm::vec3 data;
    GLsizei count = 0U;

    while (!file.eof())
    {

        std::getline(file, line);

        if (line[0] == '#')
        {
            continue;
        }

        ModelLoader::rtrim(line);
        if (line.empty())
        {
            continue;
        }

        std::istringstream stream(line);
        stream >> token;

        if (token == "mtllib")
        {

            stream >> std::ws;
            std::getline(stream, token);

            readMaterial(model_data->model_path.substr(0U, model_data->model_path.find_last_of(DIR_SEP) + 1U) + token);
        }

        else if ((token == "usemtl") && model_data->material_open)
        {

            if (!model_data->object_stock.empty())
            {
                model_data->object_stock.back()->count = static_cast<GLsizei>(index_stock.size()) - count;
                count = static_cast<GLsizei>(index_stock.size());
            }

            stream >> std::ws;
            std::getline(stream, token);

            for (Material *const material : model_data->material_stock)
            {
                if (material->getName() == token)
                {
                    model_data->object_stock.emplace_back(new ModelData::Object(0, count, material));
                    break;
                }
            }
        }

        else if (token == "v")
        {
            stream >> data.x >> data.y >> data.z;
            position_stock.emplace_back(data);

            if (data.x < model_data->min.x)
                model_data->min.x = data.x;
            if (data.y < model_data->min.y)
                model_data->min.y = data.y;
            if (data.z < model_data->min.z)
                model_data->min.z = data.z;
            if (data.x > model_data->max.x)
                model_data->max.x = data.x;
            if (data.y > model_data->max.y)
                model_data->max.y = data.y;
            if (data.z > model_data->max.z)
                model_data->max.z = data.z;
        }

        else if (token == "vn")
        {
            stream >> data.x >> data.y >> data.z;
            normal_stock.emplace_back(data);
        }

        else if (token == "vt")
        {
            stream >> data.x >> data.y;
            uv_coord_stock.emplace_back(glm::vec2(data));
        }

        else if (token == "f")
        {

            while (stream >> token)
            {
                face.emplace_back(token);
            }

            const std::string &first = *face.begin();

            for (std::vector<std::string>::iterator it = face.begin() + 2; it != face.end(); it++)
            {

                const GLsizei ind_0 = storeVertex(first);
                const GLsizei ind_1 = storeVertex(*(it - 1));

                const GLsizei ind_2 = storeVertex(*it);

                calcTangent(ind_0, ind_1, ind_2);
            }

            face.clear();
        }
    }

    file.close();

    if (model_data->material_open)
    {
        model_data->object_stock.back()->count = static_cast<GLsizei>(index_stock.size()) - count;
    }

    else
    {
        Material *material = new Material("default");
        model_data->material_stock.emplace_back(material);
        model_data->object_stock.emplace_back(new ModelData::Object(static_cast<GLsizei>(index_stock.size()), 0, material));
    }

    for (ModelLoader::Vertex &vertex : vertex_stock)
    {
        vertex.tangent = glm::normalize(vertex.tangent - vertex.normal * glm::dot(vertex.normal, vertex.tangent));
    }

    glm::vec3 dim = model_data->max - model_data->min;
    float min_dim = 1.0F / glm::max(glm::max(dim.x, dim.y), dim.z);
    model_data->origin_mat = glm::translate(glm::scale(glm::mat4(1.0F), glm::vec3(min_dim)), (model_data->min + model_data->max) / -2.0F);

    model_data->vertices = position_stock.size();
    model_data->elements = vertex_stock.size();
    model_data->triangles = index_stock.size() / 3U;

    parsed_vertex.clear();
    position_stock.clear();
    uv_coord_stock.clear();
    normal_stock.clear();

    model_data->model_open = true;
    return true;
}

bool OBJLoader::readMaterial(const std::string &mtl)
{

    model_data->material_path = mtl;
    const std::string relative = model_data->material_path.substr(0U, model_data->material_path.find_last_of(DIR_SEP) + 1U);

    std::ifstream file(model_data->material_path);
    if (!file.is_open())
    {
        return false;
    }

    Material *material = nullptr;
    bool load_cube_map = false;
    std::string cube_map_path[6];
    std::string token;
    std::string line;
    glm::vec3 data;
    float value;

    while (!file.eof())
    {

        std::getline(file, line);

        if (line[0] == '#')
        {
            continue;
        }

        ModelLoader::rtrim(line);
        if (line.empty())
        {
            continue;
        }

        std::istringstream stream(line);
        stream >> token;

        std::transform(token.begin(), token.end(), token.begin(), ::tolower);

        if (token == "newmtl")
        {

            if (load_cube_map)
            {
                material->setCubeMapTexturePath(cube_map_path);

                load_cube_map = false;
                for (int i = 0; i < 6; i++)
                {
                    cube_map_path[i].clear();
                }
            }

            stream >> std::ws;
            std::getline(stream, token);

            model_data->material_stock.emplace_back(new Material(token));
            material = model_data->material_stock.back();
        }

        else if (token == "ka")
        {
            stream >> data.x >> data.y >> data.z;
            material->setColor(Material::AMBIENT, data);
        }

        else if (token == "kd")
        {
            stream >> data.x >> data.y >> data.z;
            material->setColor(Material::DIFFUSE, data);
        }

        else if (token == "ks")
        {
            stream >> data.x >> data.y >> data.z;
            material->setColor(Material::SPECULAR, data);
        }

        else if (token == "tf")
        {
            stream >> data.x >> data.y >> data.z;
            material->setColor(Material::TRANSPARENCY, data);
        }

        else if (token == "ns")
        {
            stream >> value;
            material->setValue(Material::SHININESS, value);
        }

        else if (token == "d")
        {
            stream >> value;
            material->setValue(Material::TRANSPARENCY, 1.0F - value);
        }

        else if (token == "tr")
        {
            stream >> value;
            material->setValue(Material::TRANSPARENCY, value);
        }

        else if (token == "ni")
        {
            stream >> value;
            material->setValue(Material::REFRACTIVE_INDEX, value);
        }

        else if (token == "map_ka")
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::AMBIENT, relative + token);
            model_data->textures++;
        }

        else if (token == "map_kd")
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::DIFFUSE, relative + token);
            model_data->textures++;
        }

        else if (token == "map_ks")
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::SPECULAR, relative + token);
            model_data->textures++;
        }

        else if (token == "map_ns")
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::SHININESS, relative + token);
            model_data->textures++;
        }

        else if ((token == "map_bump") || (token == "bump") || (token == "kn"))
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::NORMAL, relative + token);
            model_data->textures++;
        }

        else if (token == "disp")
        {
            stream >> std::ws;
            std::getline(stream, token);
            material->setTexturePath(Material::DISPLACEMENT, relative + token);
            model_data->textures++;
        }

        else if (token == "refl")
        {

            bool is_cube = false;

            stream >> token;
            stream >> token;

            if (token == "cube_right")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[0] = relative + token;
                is_cube = true;
            }

            else if (token == "cube_left")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[1] = relative + token;
                is_cube = true;
            }

            else if (token == "cube_top")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[2] = relative + token;
                is_cube = true;
            }

            else if (token == "cube_bottom")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[3] = relative + token;
                is_cube = true;
            }

            else if (token == "cube_front")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[4] = relative + token;
                is_cube = true;
            }

            else if (token == "cube_back")
            {
                stream >> std::ws;
                std::getline(stream, token);
                cube_map_path[5] = relative + token;
                is_cube = true;
            }

            if (is_cube)
            {
                model_data->textures++;
                load_cube_map = true;
            }
        }
    }

    file.close();

    model_data->material_open = true;
    return true;
}

OBJLoader::OBJLoader(const std::string &path) : ModelLoader(path) {}