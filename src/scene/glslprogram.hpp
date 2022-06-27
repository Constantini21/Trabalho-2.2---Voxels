#ifndef __GLSL_PROGRAM_HPP_
#define __GLSL_PROGRAM_HPP_

#include "../glad/glad.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <string>

#include <map>

class GLSLProgram
{
private:
    GLuint program;
    std::string vert_path;
    std::string geom_path;
    std::string frag_path;

    std::size_t shaders;
    std::map<std::string, GLint> location_stock;
    GLSLProgram(const GLSLProgram &) = delete;
    GLSLProgram &operator=(const GLSLProgram &) = delete;
    GLint getUniformLocation(const GLchar *&name);
    static GLuint current_program;
    static GLuint compileShaderFile(const std::string &path, const GLenum &type);
    static GLuint compileShaderSource(const GLchar *const &source, const GLenum &type);

public:
    GLSLProgram();
    GLSLProgram(const std::string &vert, const std::string &frag);
    GLSLProgram(const std::string &vert, const std::string &geom, const std::string &frag);
    bool isValid() const;
    GLuint getProgramObject() const;
    std::string getShaderPath(const GLenum &type) const;
    std::size_t getNumberOfShaders() const;
    void setUniform(const GLchar *name, const GLint &value);
    void setUniform(const GLchar *name, const GLuint &value);
    void setUniform(const GLchar *name, const GLfloat &value);
    void setUniform(const GLchar *name, const glm::vec2 &vector);
    void setUniform(const GLchar *name, const glm::vec3 &vector);
    void setUniform(const GLchar *name, const glm::vec4 &vector);
    void setUniform(const GLchar *name, const glm::mat3 &matrix);
    void setUniform(const GLchar *name, const glm::mat4 &matrix);
    void link();
    void link(const std::string &vert, const std::string &frag);
    void link(const std::string &vert, const std::string &geom, const std::string &frag);
    void use() const;
    virtual ~GLSLProgram();
};

#endif