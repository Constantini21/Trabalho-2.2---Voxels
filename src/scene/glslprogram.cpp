#include "glslprogram.hpp"
#include <iostream>
#include <fstream>

GLuint GLSLProgram::current_program = GL_FALSE;

GLint GLSLProgram::getUniformLocation(const GLchar *&name)
{
    if ((program == GL_FALSE) || (program != GLSLProgram::current_program))
    {
        return -1;
    }

    std::map<std::string, GLint>::const_iterator result = location_stock.find(name);
    if (result != location_stock.end())
    {
        return result->second;
    }

    GLint location = glGetUniformLocation(program, name);
    location_stock[name] = location;

    return location;
}

GLuint GLSLProgram::compileShaderFile(const std::string &path, const GLenum &type)
{

    if (path.empty())
    {
        return GL_FALSE;
    }

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "error: cannot open the shader source file `" << path << "'" << std::endl;
        return GL_FALSE;
    }

    const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    GLuint shader = GLSLProgram::compileShaderSource(source.c_str(), type);
    if (shader == GL_FALSE)
    {
        std::cerr << "error: shader source path `" << path << "'" << std::endl;
    }

    return shader;
}

GLuint GLSLProgram::compileShaderSource(const GLchar *const &source, const GLenum &type)
{

    GLuint shader = glCreateShader(type);

    if (shader == GL_FALSE)
    {

        std::cerr << "error: could not create the shader object" << std::endl;

        return shader;
    }

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {

        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (length > 0)
        {

            GLchar *log = new GLchar[length];
            glGetShaderInfoLog(shader, length, nullptr, log);

            std::cerr << log;
            delete[] log;
        }

        std::cerr << "error: could not compile the shader source" << std::endl;

        glDeleteShader(shader);
        shader = GL_FALSE;
    }

    return shader;
}

GLSLProgram::GLSLProgram() : program(GL_FALSE),
                             shaders(0U) {}

GLSLProgram::GLSLProgram(const std::string &vert, const std::string &frag) :

                                                                             program(GL_FALSE),

                                                                             vert_path(vert),
                                                                             frag_path(frag),

                                                                             shaders(0U)
{

    link();
}

GLSLProgram::GLSLProgram(const std::string &vert, const std::string &geom, const std::string &frag) :

                                                                                                      program(GL_FALSE),

                                                                                                      vert_path(vert),
                                                                                                      geom_path(geom),
                                                                                                      frag_path(frag),

                                                                                                      shaders(0U)
{

    link();
}

bool GLSLProgram::isValid() const
{
    return program != GL_FALSE;
}

GLuint GLSLProgram::getProgramObject() const
{
    return program;
}

std::string GLSLProgram::getShaderPath(const GLenum &type) const
{
    switch (type)
    {

    case GL_VERTEX_SHADER:
        return vert_path;
    case GL_GEOMETRY_SHADER:
        return geom_path;
    case GL_FRAGMENT_SHADER:
        return frag_path;

    case GL_COMPUTE_SHADER:
        std::cerr << "error: the compute shader is not available for this GLSLProgram class implementation" << std::endl;
        return "NOT_AVAILABLE";

    case GL_TESS_CONTROL_SHADER:
    case GL_TESS_EVALUATION_SHADER:
        std::cerr << "error: the tessellation shader is not available for this GLSLProgram class implementation" << std::endl;
        return "NOT_AVAILABLE";

    default:
        std::cerr << "error: invalid shader type (" << type << ")" << std::endl;
        return "INVALID";
    }
}

std::size_t GLSLProgram::getNumberOfShaders() const
{
    return shaders;
}

void GLSLProgram::setUniform(const GLchar *name, const GLint &value)
{
    glUniform1i(getUniformLocation(name), value);
}

void GLSLProgram::setUniform(const GLchar *name, const GLuint &value)
{
    glUniform1ui(getUniformLocation(name), value);
}

void GLSLProgram::setUniform(const GLchar *name, const GLfloat &value)
{
    glUniform1f(getUniformLocation(name), value);
}

void GLSLProgram::setUniform(const GLchar *name, const glm::vec2 &vector)
{
    glUniform2fv(getUniformLocation(name), 1, &vector[0]);
}

void GLSLProgram::setUniform(const GLchar *name, const glm::vec3 &vector)
{
    glUniform3fv(getUniformLocation(name), 1, &vector[0]);
}

void GLSLProgram::setUniform(const GLchar *name, const glm::vec4 &vector)
{
    glUniform4fv(getUniformLocation(name), 1, &vector[0]);
}

void GLSLProgram::setUniform(const GLchar *name, const glm::mat3 &matrix)
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void GLSLProgram::setUniform(const GLchar *name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void GLSLProgram::link()
{

    if (program != GL_FALSE)
    {
        location_stock.clear();
        glDeleteProgram(program);
        program = GL_FALSE;
    }

    shaders = 0;
    bool mandatory_empty = false;

    if (vert_path.empty())
    {
        std::cerr << "error: the vertex shader source path cannot be empty" << std::endl;
        mandatory_empty = true;
    }
    else
    {
        shaders++;
    }

    if (vert_path.empty())
    {
        std::cerr << "error: the fragment shader source path cannot be empty" << std::endl;
        mandatory_empty = true;
    }
    else
    {
        shaders++;
    }

    if (!geom_path.empty())
    {
        shaders++;
    }

    if (mandatory_empty)
    {
        return;
    }

    const GLuint vert = GLSLProgram::compileShaderFile(vert_path, GL_VERTEX_SHADER);
    if (vert == GL_FALSE)
    {
        return;
    }

    const GLuint frag = GLSLProgram::compileShaderFile(frag_path, GL_FRAGMENT_SHADER);
    if (frag == GL_FALSE)
    {
        glDeleteShader(vert);
        return;
    }

    const GLuint geom = GLSLProgram::compileShaderFile(geom_path, GL_GEOMETRY_SHADER);
    if (!geom_path.empty() && (geom == GL_FALSE))
    {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return;
    }

    program = glCreateProgram();
    if (program == GL_FALSE)
    {
        std::cerr << "error: could not create the shader program object" << std::endl;

        glDeleteShader(vert);
        glDeleteShader(frag);
        glDeleteShader(geom);
        return;
    }

    glAttachShader(program, vert);
    glAttachShader(program, frag);

    if (geom != GL_FALSE)
    {
        glAttachShader(program, geom);
    }

    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    glDeleteShader(geom);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {

        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        if (length > 0)
        {

            GLchar *log = new GLchar[length];
            glGetProgramInfoLog(program, length, nullptr, log);

            std::cerr << log;
            delete[] log;
        }

        std::cerr << "error: could not link the shader program object" << std::endl;

        glDeleteProgram(program);
        program = GL_FALSE;
    }
}

void GLSLProgram::link(const std::string &vert, const std::string &frag)
{

    geom_path.clear();

    vert_path = vert;
    frag_path = frag;

    link();
}

void GLSLProgram::link(const std::string &vert, const std::string &geom, const std::string &frag)
{

    vert_path = vert;
    geom_path = geom;
    frag_path = frag;

    link();
}

void GLSLProgram::use() const
{

    if (program != GLSLProgram::current_program)
    {
        glUseProgram(program);
        GLSLProgram::current_program = program;
    }
}

GLSLProgram::~GLSLProgram()
{
    if (program != GL_FALSE)
    {
        glDeleteProgram(program);
    }
}