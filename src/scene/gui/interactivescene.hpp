#ifndef __INTERACTIVE_SCENE_HPP_
#define __INTERACTIVE_SCENE_HPP_

#include "../scene.hpp"

#include "mouse.hpp"


class InteractiveScene : public Scene {
    private:
        
        enum Focus {
            NONE,
            SCENE,
            GUI
        };

        Mouse *const mouse;
        bool cursor_enabled;
        glm::vec2 cursor_position;
        InteractiveScene::Focus focus;
        bool show_main_gui;
        bool show_metrics;
        bool show_about;
        bool show_about_imgui;
        bool focus_gui;
        
        void drawGUI();
        void showMainGUIWindow();
        bool cameraWidget(Camera *const camera, const std::size_t &id = 0U);
        bool modelWidget(std::pair<Model *, std::size_t> &model_data);
        bool lightWidget(Light *const light);
        bool programWidget(std::pair<GLSLProgram *, std::string> &program_data);
        bool programComboItem(const std::size_t &current, const std::size_t &program);
        void processKeyboardInput();
        static const std::map<Material::Attribute, std::string> AVAILABLE_TEXTURE;
        static const char *LIGHT_TYPE_LABEL[];
        static char repository_url[];

        static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
        static void mouseButtonCallback(GLFWwindow *window, int, int action, int);
        static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
        static void scrollCallback(GLFWwindow *window, double, double yoffset);
        static void keyCallback(GLFWwindow *window, int key, int, int action, int modifier);


    public:
        InteractiveScene(const std::string &title, const int &width = 800, const int &height = 600, const int &context_ver_maj = 3, const int &context_ver_min = 3);
        bool isMainGUIVisible() const;
        bool isMetricsVisible() const;
        bool isAboutVisible() const;
        bool isAboutImGuiVisible() const;
        bool isCursorEnabled() const;

        Mouse *getMouse() const;

        void setMainGUIVisible(const bool &status);
        void setMetricsVisible(const bool &status);
        void setAboutVisible(const bool &status);

        void setAboutImGuiVisible(const bool &status);
        void setCursorEnabled(const bool &status);
        void mainLoop();
        ~InteractiveScene();
        static void showAboutWindow(bool &show);
};

#endif 