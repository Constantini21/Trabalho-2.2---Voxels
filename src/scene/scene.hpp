#ifndef __SCENE_HPP_
#define __SCENE_HPP_

#include "camera.hpp"
#include "../model/model.hpp"
#include "light.hpp"
#include "glslprogram.hpp"
#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <string>
#include <map>
#include <forward_list>

class Scene
{
protected:
    GLFWwindow *window;

    std::string title;
    int width;
    int height;

    glm::vec3 background_color;
    Camera *active_camera;
    std::map<std::size_t, Camera *> camera_stock;

    std::map<std::size_t, std::pair<Model *, std::size_t>> model_stock;
    std::map<std::size_t, Light *> light_stock;
    std::size_t lighting_program;
    std::map<std::size_t, std::pair<GLSLProgram *, std::string>> program_stock;
    double kframes;

    Scene() = delete;

    Scene(const Scene &) = delete;

    Scene &operator=(const Scene &) = delete;

    void drawScene();
    static std::size_t instances;
    static std::size_t element_id;
    static bool initialized_glad;
    static GLsizei screen_width;
    static GLsizei screen_height;
    static GLuint square_vao;
    static GLuint square_vbo;
    static GLuint fbo;
    static GLuint rbo;
    static GLuint buffer_texture[];
    static const GLubyte *opengl_vendor;
    static const GLubyte *opengl_renderer;

    static const GLubyte *opengl_version;
    static const GLubyte *glsl_version;

    static void createGeometryFrameBuffer();

    static void attachTextureToFrameBuffer(const GLenum &attachment, const GLint &internalFormat, const GLenum &format, const GLenum &type);
    static void createSquare();
    static void errorCallback(int error, const char *description);
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

public:
    Scene(const std::string &title, const int &width = 800, const int &height = 600, const int &context_ver_maj = 3, const int &context_ver_min = 3);
    bool isValid();
    std::string getTitle() const;
    glm::vec2 getResolution() const;
    glm::vec3 getBackgroundColor() const;
    Camera *getActiveCamera() const;
    Camera *getCamera(const std::size_t &id = 0U) const;
    Model *getModel(const std::size_t &id) const;
    Light *getLight(const std::size_t &id) const;
    std::size_t getModelProgram(const std::size_t &id) const;
    GLSLProgram *getProgram(const std::size_t &id) const;
    std::string getProgramDescription(const std::size_t &id) const;
    std::size_t getLightingPassProgramID() const;
    GLSLProgram *getDefaultGeometryPassProgram();
    std::string getDefaultGeometryPassProgramDescription();
    GLSLProgram *getDefaultLightingPassProgram();
    std::string getDefaultLightingPassProgramDescription();
    double getFrames() const;
    void setBackgroundColor(const glm::vec3 &color);
    bool selectCamera(const std::size_t &id);
    std::size_t addCamera(const bool &orthogonal = false);
    std::size_t addModel();
    std::size_t addModel(const std::string &path, const std::size_t &program_id = 0U);
    std::size_t addLight(const Light::Type &type = Light::DIRECTIONAL);
    std::size_t addProgram(const std::string &desc);
    std::size_t addProgram(const std::string &desc, const std::string &vert, const std::string &frag);
    std::size_t addProgram(const std::string &desc, const std::string &vert, const std::string &geom, const std::string &frag);
    bool setProgramDescription(const std::string &desc, const std::size_t &id);
    void setLightingPassProgram(const std::size_t &id);
    void setDefaultGeometryPassProgram(const std::string &desc, const std::string &vert, const std::string &frag);
    void setDefaultGeometryPassProgramDescription(const std::string &desc);
    void setDefaultLightingPassProgram(const std::string &desc, const std::string &vert, const std::string &frag);
    void setDefaultLightingPassProgramDescription(const std::string &desc);
    void setTitle(const std::string &new_title);
    std::size_t setProgramToModel(const std::size_t &program_id, const std::size_t &model_id);
    virtual void mainLoop();
    void reloadPrograms();
    bool removeCamera(const std::size_t &id);
    bool removeModel(const std::size_t &id);
    bool removeLight(const std::size_t &id);
    bool removeProgram(const std::size_t &id);
    void removeDefaultGeometryPassProgram();
    void removeDefaultLightingPassProgram();
    virtual ~Scene();
    static const GLubyte *getOpenGLVendor();
    static const GLubyte *getOpenGLRenderer();
    static const GLubyte *getOpenGLVersion();
    static const GLubyte *getGLSLVersion();
};

#endif // __SCENE_HPP_