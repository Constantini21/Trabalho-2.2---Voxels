#include "material.hpp"
#define STBI_ASSERT(x)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <iostream>

const Material::Attribute Material::TEXTURE_ATTRIBUTE[] = {
    Material::AMBIENT,
    Material::DIFFUSE,
    Material::SPECULAR,
    Material::SHININESS,
    Material::NORMAL,
    Material::DISPLACEMENT};

GLuint Material::default_texture[3] = {GL_FALSE, GL_FALSE, GL_FALSE};

GLuint Material::createDefaultTexture(const GLubyte *const color)
{
    float border[4];
    for (int i = 0; i < 3; i++)
    {
        border[i] = (color[i] != 0.0F ? 255.0F / color[i] : 0.0F);
    }
    border[3] = 1.0F;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &color[0]);

    return texture;
}

void Material::bindTexture(const GLenum &index, const GLuint &texture)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

GLuint Material::load2DTexture(const std::string &path)
{

    if (path.empty())
    {
        return GL_FALSE;
    }

    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(true);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr)
    {
        std::cerr << "error: could not open the texture `" << path << "'" << std::endl;
        return GL_FALSE;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}

GLuint Material::loadCubeMapTexture(const std::string (&path)[6])
{
    int width[6] = {1};
    int height[6] = {1};
    int channels[6];

    stbi_set_flip_vertically_on_load(true);
    stbi_uc *data[6];

    for (int i = 0; i < 6; i++)
    {
        data[i] = stbi_load(path[i].c_str(), &width[i], &height[i], &channels[i], STBI_rgb);

        if (data[i] == nullptr)
        {
            std::cerr << "error: could not open the texture `" << path << "' (cube side " << i << ")" << std::endl;
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    for (GLint i = 0; i < 6; i++)
    {
        if (data[i] != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
            stbi_image_free(data[i]);
        }
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return texture;
}

Material::Material(const std::string &name) :

                                              name(name),

                                              color{glm::vec3(0.0F), glm::vec3(1.0F), glm::vec3(0.125F), glm::vec3(1.0F)},

                                              value{96.078431F, 0.3F, 0.3F, 0.0F, 0.05, 1.0F},

                                              texture{GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE},

                                              texture_enabled{true, true, true, true, true, true, true}
{
}

std::string Material::getName() const
{
    return name;
}

glm::vec3 Material::getColor(const Material::Attribute &attrib) const
{
    switch (attrib)
    {

    case Material::AMBIENT:
        return color[0];
    case Material::DIFFUSE:
        return color[1];
    case Material::SPECULAR:
        return color[2];
    case Material::TRANSPARENCY:
        return color[3];

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return glm::vec3(NAN);
    }
}

float Material::getValue(const Material::Attribute &attrib) const
{
    switch (attrib)
    {

    case Material::SHININESS:
        return value[0];
    case Material::ROUGHNESS:
        return value[1];
    case Material::METALNESS:
        return value[2];
    case Material::TRANSPARENCY:
        return value[3];
    case Material::DISPLACEMENT:
        return value[4];
    case Material::REFRACTIVE_INDEX:
        return value[5];

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return NAN;
    }
}

GLuint Material::getTexture(const Material::Attribute &attrib) const
{
    switch (attrib)
    {
    case Material::AMBIENT:
        return texture[0] == GL_FALSE ? Material::default_texture[0] : texture[0];
    case Material::DIFFUSE:
        return texture[1] == GL_FALSE ? Material::default_texture[0] : texture[1];
    case Material::SPECULAR:
        return texture[2] == GL_FALSE ? Material::default_texture[0] : texture[2];
    case Material::SHININESS:
        return texture[3] == GL_FALSE ? Material::default_texture[0] : texture[3];
    case Material::NORMAL:
        return texture[4] == GL_FALSE ? Material::default_texture[1] : texture[4];
    case Material::DISPLACEMENT:
        return texture[5] == GL_FALSE ? Material::default_texture[2] : texture[5];

    case Material::CUBE_MAP:
    case Material::CUBE_MAP_RIGHT:
    case Material::CUBE_MAP_LEFT:
    case Material::CUBE_MAP_TOP:
    case Material::CUBE_MAP_BOTTOM:
    case Material::CUBE_MAP_FRONT:
    case Material::CUBE_MAP_BACK:
        return texture[6];

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return GL_FALSE;
    }
}

bool Material::isTextureEnabled(const Material::Attribute &attrib) const
{
    switch (attrib)
    {
    case Material::AMBIENT:
        return texture_enabled[0];
    case Material::DIFFUSE:
        return texture_enabled[1];
    case Material::SPECULAR:
        return texture_enabled[2];
    case Material::SHININESS:
        return texture_enabled[3];
    case Material::NORMAL:
        return texture_enabled[4];
    case Material::DISPLACEMENT:
        return texture_enabled[5];

    case Material::CUBE_MAP:
    case Material::CUBE_MAP_RIGHT:
    case Material::CUBE_MAP_LEFT:
    case Material::CUBE_MAP_TOP:
    case Material::CUBE_MAP_BOTTOM:
    case Material::CUBE_MAP_FRONT:
    case Material::CUBE_MAP_BACK:
        return texture_enabled[6];

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return false;
    }
}

std::string Material::getTexturePath(const Material::Attribute &attrib) const
{
    switch (attrib)
    {
    case Material::AMBIENT:
        return texture_path[0];
    case Material::DIFFUSE:
        return texture_path[1];
    case Material::SPECULAR:
        return texture_path[2];
    case Material::SHININESS:
        return texture_path[3];
    case Material::NORMAL:
        return texture_path[4];
    case Material::DISPLACEMENT:
        return texture_path[5];

    case Material::CUBE_MAP_RIGHT:
        return texture_path[6];
    case Material::CUBE_MAP_LEFT:
        return texture_path[7];
    case Material::CUBE_MAP_TOP:
        return texture_path[8];
    case Material::CUBE_MAP_BOTTOM:
        return texture_path[9];
    case Material::CUBE_MAP_FRONT:
        return texture_path[10];
    case Material::CUBE_MAP_BACK:
        return texture_path[11];

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return "INVALID";
    }
}

void Material::setName(const std::string &new_name)
{
    name = new_name;
}

void Material::setColor(const Material::Attribute &attrib, const glm::vec3 &new_color)
{
    switch (attrib)
    {
    case Material::AMBIENT:
        color[0] = new_color;
        return;
    case Material::DIFFUSE:
        color[1] = new_color;
        return;
    case Material::SPECULAR:
        color[2] = new_color;
        return;
    case Material::TRANSPARENCY:
        color[3] = new_color;
        return;

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
    }
}

void Material::setValue(const Material::Attribute &attrib, const float &new_value)
{
    switch (attrib)
    {
    case Material::SHININESS:
        value[0] = new_value;
        return;
    case Material::ROUGHNESS:
        value[1] = new_value;
        return;
    case Material::METALNESS:
        value[2] = new_value;
        return;
    case Material::TRANSPARENCY:
        value[3] = new_value;
        return;
    case Material::DISPLACEMENT:
        value[4] = new_value;
        return;
    case Material::REFRACTIVE_INDEX:
        value[5] = new_value;
        return;

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
    }
}

void Material::setTextureEnabled(const Material::Attribute &attrib, const bool &status)
{
    switch (attrib)
    {
    case Material::AMBIENT:
        texture_enabled[0] = status;
        return;
    case Material::DIFFUSE:
        texture_enabled[1] = status;
        return;
    case Material::SPECULAR:
        texture_enabled[2] = status;
        return;
    case Material::SHININESS:
        texture_enabled[3] = status;
        return;
    case Material::NORMAL:
        texture_enabled[4] = status;
        return;
    case Material::DISPLACEMENT:
        texture_enabled[5] = status;
        return;

    case Material::CUBE_MAP:
    case Material::CUBE_MAP_RIGHT:
    case Material::CUBE_MAP_LEFT:
    case Material::CUBE_MAP_TOP:
    case Material::CUBE_MAP_BOTTOM:
    case Material::CUBE_MAP_FRONT:
    case Material::CUBE_MAP_BACK:
        texture_enabled[6] = status;
        return;

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
    }
}

void Material::setTexturePath(const Material::Attribute &attrib, const std::string &path)
{
    switch (attrib)
    {
    case Material::AMBIENT:
        texture_path[0] = path;
        break;
    case Material::DIFFUSE:
        texture_path[1] = path;
        break;
    case Material::SPECULAR:
        texture_path[2] = path;
        break;
    case Material::SHININESS:
        texture_path[3] = path;
        break;
    case Material::NORMAL:
        texture_path[4] = path;
        break;
    case Material::DISPLACEMENT:
        texture_path[5] = path;
        break;

    case Material::CUBE_MAP:
    case Material::CUBE_MAP_RIGHT:
    case Material::CUBE_MAP_LEFT:
    case Material::CUBE_MAP_TOP:
    case Material::CUBE_MAP_BOTTOM:
    case Material::CUBE_MAP_FRONT:
    case Material::CUBE_MAP_BACK:

        std::cout << "warning: using the same texture for all cube map sides, use `Material::setCubeMapTexturePath(const std::string (&path)[6])' to set each cube map path separately" << std::endl;

        for (int i = 6; i < 12; i++)
        {
            texture_path[i] = path;
        }

        break;

    default:
        std::cerr << "error: invalid attribute `" << attrib << "'" << std::endl;
        return;
    }

    reloadTexture(attrib);
}

void Material::setCubeMapTexturePath(const std::string (&path)[6])
{
    for (int i = 6, j = 1; j < 6; j++)
    {
        texture_path[i] = path[j];
    }

    reloadTexture(Material::CUBE_MAP);
}

void Material::reloadTexture(const Material::Attribute &attrib)
{
    for (int i = 0; i < 6; i++)
    {
        if (attrib & Material::TEXTURE_ATTRIBUTE[i])
        {
            glDeleteTextures(1, &texture[i]);
            texture[i] = Material::load2DTexture(texture_path[i]);
        }
    }

    if (attrib & Material::CUBE_MAP)
    {
        glDeleteTextures(1, &texture[6]);
        texture[6] = Material::loadCubeMapTexture({texture_path[6], texture_path[7], texture_path[8], texture_path[9], texture_path[10], texture_path[11]});
    }
}

void Material::bind(GLSLProgram *const program) const
{
    if ((program == nullptr) || (!program->isValid()))
    {
        return;
    }

    program->use();
    program->setUniform("u_ambient", color[0]);
    program->setUniform("u_diffuse", color[1]);
    program->setUniform("u_specular", color[2]);
    program->setUniform("u_transmision", color[3]);

    program->setUniform("u_shininess", value[0]);
    program->setUniform("u_roughness", value[1]);
    program->setUniform("u_metalness", value[2]);
    program->setUniform("u_alpha", (1.0F - value[3]));
    program->setUniform("u_displacement", value[4]);
    program->setUniform("u_refractive_index", value[5]);

    program->setUniform("u_ambient_tex", 0);
    program->setUniform("u_diffuse_tex", 1);
    program->setUniform("u_specular_tex", 2);
    program->setUniform("u_shininess_tex", 3);
    program->setUniform("u_normal_tex", 4);
    program->setUniform("u_displacement_tex", 5);
    program->setUniform("u_cube_map_tex", 6);

    Material::bindTexture(0U, (texture[0] == GL_FALSE) || !texture_enabled[0] ? Material::default_texture[0] : texture[0]);
    Material::bindTexture(1U, (texture[1] == GL_FALSE) || !texture_enabled[1] ? Material::default_texture[0] : texture[1]);
    Material::bindTexture(2U, (texture[2] == GL_FALSE) || !texture_enabled[2] ? Material::default_texture[0] : texture[2]);
    Material::bindTexture(3U, (texture[3] == GL_FALSE) || !texture_enabled[3] ? Material::default_texture[0] : texture[3]);
    Material::bindTexture(4U, (texture[4] == GL_FALSE) || !texture_enabled[4] ? Material::default_texture[1] : texture[4]);
    Material::bindTexture(5U, (texture[5] == GL_FALSE) || !texture_enabled[5] ? Material::default_texture[2] : texture[5]);
    Material::bindTexture(6U, texture[6]);
}

Material::~Material()
{
    glDeleteTextures(6, &texture[0]);
}

void Material::createDefaultTextures()
{
    GLubyte color[3];

    if (Material::default_texture[0] == GL_FALSE)
    {
        color[0] = 255U;
        color[1] = 255U;
        color[2] = 255U;
        Material::default_texture[0] = Material::createDefaultTexture(color);
    }

    if (Material::default_texture[1] == GL_FALSE)
    {
        color[0] = 0U;
        color[1] = 0U;
        color[2] = 255U;
        Material::default_texture[1] = Material::createDefaultTexture(color);
    }

    if (Material::default_texture[2] == GL_FALSE)
    {
        color[0] = 0U;
        color[1] = 0U;
        color[2] = 0U;
        Material::default_texture[2] = Material::createDefaultTexture(color);
    }
}

void Material::deleteDefaultTextures()
{
    glDeleteTextures(3, &Material::default_texture[0]);

    Material::default_texture[0] = GL_FALSE;
    Material::default_texture[1] = GL_FALSE;
    Material::default_texture[2] = GL_FALSE;
}