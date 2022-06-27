#ifndef __MATERIAL_HPP_
#define __MATERIAL_HPP_

#include "../scene/glslprogram.hpp"
#include "../glad/glad.h"
#include <glm/vec3.hpp>
#include <string>

class Material {
    public:
        enum Attribute : GLenum {
            AMBIENT = 0x0001,
            DIFFUSE = 0x0002,
            SPECULAR = 0x0004,
            SHININESS = 0x0008,
            ROUGHNESS = 0x0010,
            METALNESS = 0x0020,
            TRANSPARENCY = 0x0040,
            NORMAL = 0x0080,
            DISPLACEMENT = 0x0100,
            REFRACTIVE_INDEX = 0x0200,
            CUBE_MAP = 0x8000,
            CUBE_MAP_RIGHT = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            CUBE_MAP_LEFT = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            CUBE_MAP_TOP = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            CUBE_MAP_BOTTOM = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            CUBE_MAP_FRONT = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            CUBE_MAP_BACK = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
            ALL_TEXTURES = 0x818F
        };

    private:
        std::string name;
        glm::vec3 color[4];
        float value[6];
        GLuint texture[7];
        bool texture_enabled[7];
        std::string texture_path[12];
        Material() = delete;
        Material(const Material &) = delete;
        Material &operator=(const Material &) = delete;
        static const Material::Attribute TEXTURE_ATTRIBUTE[];
        static GLuint default_texture[3];
        static GLuint createDefaultTexture(const GLubyte *const color);
        static void bindTexture(const GLenum &index, const GLuint &texture);
        static GLuint load2DTexture(const std::string &path);
        static GLuint loadCubeMapTexture(const std::string (&path)[6]);

    public:
        Material(const std::string &name);
        std::string getName() const;
        glm::vec3 getColor(const Material::Attribute &attrib) const;
        float getValue(const Material::Attribute &attrib) const;
        GLuint getTexture(const Material::Attribute &attrib) const;
        bool isTextureEnabled(const Material::Attribute &attrib) const;
        std::string getTexturePath(const Material::Attribute &attrib) const;
        void setName(const std::string &new_name);
        void setColor(const Material::Attribute &attrib, const glm::vec3 &new_color);
        void setValue(const Material::Attribute &attrib, const float &new_value);
        void setTextureEnabled(const Material::Attribute &attrib, const bool &status);
        void setTexturePath(const Material::Attribute &attrib, const std::string &path);
        void setCubeMapTexturePath(const std::string (&path)[6]);
        void reloadTexture(const Material::Attribute &attrib);
        void bind(GLSLProgram *const program) const;
        virtual ~Material();
        static void createDefaultTextures();
        static void deleteDefaultTextures();
};

#endif 