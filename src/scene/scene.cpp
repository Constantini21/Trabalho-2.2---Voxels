#include "scene.hpp"

#include <iostream>

#define TEXTURE_BUFFERS 6
std::size_t Scene::instances = 0U;
std::size_t Scene::element_id = 1U;
bool Scene::initialized_glad = false;
GLsizei Scene::screen_width = 0U;
GLsizei Scene::screen_height = 0U;
GLuint Scene::square_vao = GL_FALSE;
GLuint Scene::square_vbo = GL_FALSE;
GLuint Scene::fbo = GL_FALSE;
GLuint Scene::rbo = GL_FALSE;
GLuint Scene::buffer_texture[TEXTURE_BUFFERS];
const GLubyte *Scene::opengl_vendor = nullptr;
const GLubyte *Scene::opengl_renderer = nullptr;
const GLubyte *Scene::opengl_version = nullptr;
const GLubyte *Scene::glsl_version = nullptr;

void Scene::createGeometryFrameBuffer()
{
    glGenFramebuffers(1, &Scene::fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, Scene::fbo);
    glGenTextures(TEXTURE_BUFFERS, Scene::buffer_texture);

    Scene::attachTextureToFrameBuffer(0, GL_RGB16F, GL_RGB, GL_FLOAT);

    Scene::attachTextureToFrameBuffer(1, GL_RGB16F, GL_RGB, GL_FLOAT);

    Scene::attachTextureToFrameBuffer(2, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);

    Scene::attachTextureToFrameBuffer(3, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);

    Scene::attachTextureToFrameBuffer(4, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);

    Scene::attachTextureToFrameBuffer(5, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    GLenum attachment[TEXTURE_BUFFERS];
    for (GLuint i = 0; i < TEXTURE_BUFFERS; i++)
    {
        attachment[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(TEXTURE_BUFFERS, attachment);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Scene::screen_width, Scene::screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "error: the geometry frame buffer object status is not complete (" << status << ")" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::attachTextureToFrameBuffer(const GLenum &attachment, const GLint &internalFormat, const GLenum &format, const GLenum &type)
{
    glBindTexture(GL_TEXTURE_2D, Scene::buffer_texture[attachment]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Scene::screen_width, Scene::screen_height, 0, format, type, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, Scene::buffer_texture[attachment], 0);
}

void Scene::createSquare()
{

    const float data[] = {

        -1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
        -1.0F, -1.0F, 0.0F, 0.0F, 0.0F,
        1.0F, 1.0F, 0.0F, 1.0F, 1.0F,
        1.0F, -1.0F, 0.0F, 1.0F, 0.0F};

    glGenVertexArrays(1, &Scene::square_vao);
    glBindVertexArray(Scene::square_vao);

    glGenBuffers(1, &Scene::square_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Scene::square_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));

    glBindVertexArray(GL_FALSE);
}

void Scene::errorCallback(int error, const char *description)
{
    std::cerr << "error " << error << ": " << description << std::endl;
}

void Scene::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{

    Scene *const scene = static_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->width = width;
    scene->height = height;

    const glm::vec2 resolution(width, height);
    for (const std::pair<const std::size_t, Camera *const> &camera_data : scene->camera_stock)
    {
        camera_data.second->setResolution(resolution);
    }
}

void Scene::drawScene()
{
    GLSLProgram *program;

    glBindFramebuffer(GL_FRAMEBUFFER, Scene::fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screen_width, screen_height);

    for (const std::pair<const std::size_t, std::pair<const Model *const, const std::size_t>> model_data : model_stock)
    {
        if (!model_data.second.first->isOpen())
        {
            continue;
        }

        std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(model_data.second.second);
        program = (result == program_stock.end() ? program_stock[0U] : result->second).first;
        active_camera->bind(program);
        model_data.second.first->draw(program);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(lighting_program);

    program = (result == program_stock.end() ? program_stock[1U] : result->second).first;
    program->use();
    program->setUniform("u_view_pos", active_camera->getPosition());
    program->setUniform("u_position_tex", 0);
    program->setUniform("u_normal_tex", 1);
    program->setUniform("u_ambient_tex", 2);
    program->setUniform("u_diffuse_tex", 3);
    program->setUniform("u_specular_tex", 4);
    program->setUniform("u_metadata_tex", 5);

    for (GLenum i = 0; i < TEXTURE_BUFFERS; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, Scene::buffer_texture[i]);
    }

    glBindVertexArray(Scene::square_vao);

    int pass = 0;
    for (const std::pair<const std::size_t, const Light *const> &light_data : light_stock)
    {
        if (pass < 2)
        {
            program->setUniform("u_background_color", pass == 0 ? background_color : glm::vec3(0.0F));
            pass++;
        }

        light_data.second->bind(program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(GL_FALSE);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

Scene::Scene(const std::string &title, const int &width, const int &height, const int &context_ver_maj, const int &context_ver_min) : window(nullptr),
                                                                                                                                      title(title),
                                                                                                                                      width(width),
                                                                                                                                      height(height),

                                                                                                                                      background_color(0.0F),
                                                                                                                                      active_camera(nullptr),
                                                                                                                                      lighting_program(1U)
{

    bool create_window = true;

    if (Scene::instances == 0U)
    {
        glfwSetErrorCallback(Scene::errorCallback);

        if (glfwInit() != GLFW_TRUE)
        {
            std::cerr << "error: cannot initialize GLFW" << std::endl;
            create_window = false;
        }
    }

    if (create_window)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, context_ver_maj);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, context_ver_min);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        active_camera = new Camera(width, height);
        camera_stock[Scene::element_id++] = active_camera;

        light_stock[Scene::element_id++] = new Light();
    }

    if (window == nullptr)
    {
        std::cerr << "error: cannot create the application window" << std::endl;
    }

    else
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, Scene::framebufferSizeCallback);

        glfwMaximizeWindow(window);
        glfwMakeContextCurrent(window);

        if (!Scene::initialized_glad && (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0))
        {
            std::cerr << "error: cannot initialize glad" << std::endl;

            glfwDestroyWindow(window);
            window = nullptr;
        }

        else
        {
            Scene::initialized_glad = true;
            Scene::opengl_vendor = glGetString(GL_VENDOR);
            Scene::opengl_renderer = glGetString(GL_RENDERER);
            Scene::opengl_version = glGetString(GL_VERSION);
            Scene::glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

            glfwSwapInterval(1);
            glEnable(GL_DEPTH_TEST);

            program_stock[0U] = std::pair<GLSLProgram *, std::string>(new GLSLProgram(), "Empty (Default geometry pass)");
            program_stock[1U] = std::pair<GLSLProgram *, std::string>(new GLSLProgram(), "Empty (Default lighting pass)");
        }
    }

    if ((Scene::instances == 0U) && Scene::initialized_glad)
    {
        const GLFWvidmode *const video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        Scene::screen_width = video_mode->width;
        Scene::screen_height = video_mode->height;
        Scene::createSquare();
        Scene::createGeometryFrameBuffer();

        Material::createDefaultTextures();
    }

    Scene::instances++;
}

bool Scene::isValid()
{
    return window != nullptr;
}

std::string Scene::getTitle() const
{
    return title;
}

glm::vec2 Scene::getResolution() const
{
    return glm::vec2(width, height);
}

glm::vec3 Scene::getBackgroundColor() const
{
    return background_color;
}

Camera *Scene::getActiveCamera() const
{
    return active_camera;
}

Camera *Scene::getCamera(const std::size_t &id) const
{
    std::map<std::size_t, Camera *>::const_iterator result = camera_stock.find(id);
    return result == camera_stock.end() ? nullptr : result->second;
}

Model *Scene::getModel(const std::size_t &id) const
{
    std::map<std::size_t, std::pair<Model *, std::size_t>>::const_iterator result = model_stock.find(id);
    return result == model_stock.end() ? nullptr : result->second.first;
}

Light *Scene::getLight(const std::size_t &id) const
{
    std::map<std::size_t, Light *>::const_iterator result = light_stock.find(id);
    return result == light_stock.end() ? nullptr : result->second;
}

std::size_t Scene::getModelProgram(const std::size_t &id) const
{
    std::map<std::size_t, std::pair<Model *, std::size_t>>::const_iterator result = model_stock.find(id);
    return result == model_stock.end() ? 0U : result->second.second;
}

GLSLProgram *Scene::getProgram(const std::size_t &id) const
{
    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(id);
    return result == program_stock.end() ? nullptr : result->second.first;
}

std::string Scene::getProgramDescription(const std::size_t &id) const
{
    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(id);
    return result == program_stock.end() ? "NOT_FOUND" : result->second.second;
}

std::size_t Scene::getLightingPassProgramID() const
{
    return lighting_program;
}

GLSLProgram *Scene::getDefaultGeometryPassProgram()
{
    return program_stock[0U].first;
}

std::string Scene::getDefaultGeometryPassProgramDescription()
{
    return program_stock[0U].second;
}

GLSLProgram *Scene::getDefaultLightingPassProgram()
{
    return program_stock[1U].first;
}

std::string Scene::getDefaultLightingPassProgramDescription()
{
    return program_stock[1U].second;
}

double Scene::getFrames() const
{
    return kframes;
}

void Scene::setBackgroundColor(const glm::vec3 &color)
{
    background_color = color;
}

bool Scene::selectCamera(const std::size_t &id)
{
    std::map<std::size_t, Camera *>::const_iterator result = camera_stock.find(id);

    if (result == camera_stock.end())
    {
        return false;
    }

    active_camera = result->second;
    return true;
}

std::size_t Scene::addCamera(const bool &orthogonal)
{
    camera_stock[Scene::element_id] = new Camera(width, height, orthogonal);
    return Scene::element_id++;
}

std::size_t Scene::addModel()
{
    model_stock[Scene::element_id] = std::pair<Model *, std::size_t>(new Model(), 0U);
    return Scene::element_id++;
}

std::size_t Scene::addModel(const std::string &path, const std::size_t &program_id)
{
    model_stock[Scene::element_id] = std::pair<Model *, std::size_t>(new Model(path), program_id);
    return Scene::element_id++;
}

std::size_t Scene::addLight(const Light::Type &type)
{
    light_stock[Scene::element_id] = new Light(type);
    return Scene::element_id++;
}

std::size_t Scene::addProgram(const std::string &desc)
{
    program_stock[Scene::element_id] = std::pair<GLSLProgram *, std::string>(new GLSLProgram(), desc);
    return Scene::element_id++;
}

std::size_t Scene::addProgram(const std::string &desc, const std::string &vert, const std::string &frag)
{
    program_stock[Scene::element_id] = std::pair<GLSLProgram *, std::string>(new GLSLProgram(vert, frag), desc);
    return Scene::element_id++;
}

std::size_t Scene::addProgram(const std::string &desc, const std::string &vert, const std::string &geom, const std::string &frag)
{
    program_stock[Scene::element_id] = std::pair<GLSLProgram *, std::string>(new GLSLProgram(vert, geom, frag), desc);
    return Scene::element_id++;
}

bool Scene::setProgramDescription(const std::string &desc, const std::size_t &id)
{
    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::iterator result = program_stock.find(id);

    if (result == program_stock.end())
    {
        return false;
    }

    result->second.second = desc;
    return true;
}

void Scene::setLightingPassProgram(const std::size_t &id)
{
    lighting_program = id;
}

void Scene::setDefaultGeometryPassProgram(const std::string &desc, const std::string &vert, const std::string &frag)
{
    program_stock[0U].first->link(vert, frag);
    program_stock[0U].second = desc + " (Default geometry pass)";
}

void Scene::setDefaultGeometryPassProgramDescription(const std::string &desc)
{
    program_stock[0U].second = desc;
}

void Scene::setDefaultLightingPassProgram(const std::string &desc, const std::string &vert, const std::string &frag)
{
    program_stock[1U].first->link(vert, frag);
    program_stock[1U].second = desc + " (Default lighting pass)";
}

void Scene::setDefaultLightingPassProgramDescription(const std::string &desc)
{
    program_stock[1U].second = desc;
}

void Scene::setTitle(const std::string &new_title)
{
    title = new_title;
    glfwSetWindowTitle(window, title.c_str());
}

std::size_t Scene::setProgramToModel(const std::size_t &program_id, const std::size_t &model_id)
{
    std::map<std::size_t, std::pair<Model *, std::size_t>>::iterator result = model_stock.find(model_id);

    if (result == model_stock.end())
    {
        std::cerr << "error: could not found the model with id " << model_id << std::endl;
        return 0U;
    }

    const std::size_t previous_program = result->second.second;
    result->second.second = program_id;
    return previous_program;
}

void Scene::mainLoop()
{
    if (window == nullptr)
    {
        std::cerr << "error: there is no window" << std::endl;
        return;
    }

    if (!program_stock[0U].first->isValid())
    {
        std::cerr << "warning: the default geometry pass program has not been set or is not valid" << std::endl;
    }

    if (!program_stock[1U].first->isValid())
    {
        std::cerr << "warning: the default lighting pass program has not been set or is not valid" << std::endl;
    }

    while (glfwWindowShouldClose(window) == GLFW_FALSE)
    {

        drawScene();

        glfwPollEvents();
        glfwSwapBuffers(window);

        kframes += 0.001;
    }
}

void Scene::reloadPrograms()
{
    for (std::pair<const std::size_t, std::pair<GLSLProgram *, std::string>> &program_data : program_stock)
    {
        program_data.second.first->link();
    }
}

bool Scene::removeCamera(const std::size_t &id)
{

    if (camera_stock.size() == 1U)
    {
        std::cerr << "error: the camera stock could not be empty" << std::endl;
        return false;
    }

    std::map<std::size_t, Camera *>::const_iterator result = camera_stock.find(id);

    if (result == camera_stock.end())
    {
        return false;
    }

    if (result->second == active_camera)
    {
        active_camera = std::next(result, result->second == camera_stock.rbegin()->second ? -1 : 1)->second;
    }

    delete result->second;
    camera_stock.erase(result);

    return true;
}

bool Scene::removeModel(const std::size_t &id)
{

    std::map<std::size_t, std::pair<Model *, std::size_t>>::const_iterator result = model_stock.find(id);

    if (result == model_stock.end())
    {
        return false;
    }

    delete result->second.first;
    model_stock.erase(result);

    return true;
}

bool Scene::removeLight(const std::size_t &id)
{

    if (light_stock.size() == 1U)
    {
        std::cerr << "error: the light stock could not be empty" << std::endl;
        return false;
    }

    std::map<std::size_t, Light *>::const_iterator result = light_stock.find(id);

    if (result == light_stock.end())
    {
        return false;
    }

    delete result->second;
    light_stock.erase(result);

    return true;
}

bool Scene::removeProgram(const std::size_t &id)
{

    if ((id == 0U) || (id == 1U))
    {
        std::cerr << "error: cannot remove a default program" << std::endl;
        return false;
    }

    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(id);

    if (result == program_stock.end())
    {
        return false;
    }

    delete result->second.first;
    program_stock.erase(result);

    return true;
}

void Scene::removeDefaultGeometryPassProgram()
{

    std::pair<GLSLProgram *, std::string> &program_data = program_stock[0U];

    delete program_data.first;

    program_data.first = new GLSLProgram();
    program_data.second = "Empty (Default geometry pass)";
}

void Scene::removeDefaultLightingPassProgram()
{

    std::pair<GLSLProgram *, std::string> &program_data = program_stock[1U];

    delete program_data.first;

    program_data.first = new GLSLProgram();
    program_data.second = "Empty (Default lighting pass)";
}

Scene::~Scene()
{

    for (const std::pair<const std::size_t, const Camera *const> &camera_data : camera_stock)
    {
        delete camera_data.second;
    }

    for (const std::pair<const std::size_t, std::pair<const Model *const, std::size_t>> &model_data : model_stock)
    {
        delete model_data.second.first;
    }

    for (const std::pair<const std::size_t, std::pair<const GLSLProgram *const, const std::string>> &program_data : program_stock)
    {
        delete program_data.second.first;
    }

    if (window != nullptr)
    {
        glfwDestroyWindow(window);
    }

    if ((Scene::instances == 1U) && Scene::initialized_glad)
    {

        glDeleteTextures(TEXTURE_BUFFERS, Scene::buffer_texture);
        glDeleteRenderbuffers(1, &Scene::rbo);
        glDeleteFramebuffers(1, &Scene::fbo);

        glDeleteBuffers(1, &Scene::square_vbo);
        glDeleteBuffers(1, &Scene::square_vao);

        Material::deleteDefaultTextures();

        glfwTerminate();

        Scene::initialized_glad = false;

        Scene::opengl_vendor = nullptr;
        Scene::opengl_renderer = nullptr;
        Scene::opengl_version = nullptr;
        Scene::glsl_version = nullptr;
    }

    Scene::instances--;
}

const GLubyte *Scene::getOpenGLVendor()
{
    return Scene::opengl_vendor;
}

const GLubyte *Scene::getOpenGLRenderer()
{
    return Scene::opengl_renderer;
}

const GLubyte *Scene::getOpenGLVersion()
{
    return Scene::opengl_version;
}

const GLubyte *Scene::getGLSLVersion()
{
    return Scene::glsl_version;
}