#include "interactivescene.hpp"

#include "customwidgets.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

#include <cstring>

const std::map<Material::Attribute, std::string> InteractiveScene::AVAILABLE_TEXTURE = {
    {Material::AMBIENT, "Ambient"},
    {Material::DIFFUSE, "Diffuse"},
    {Material::SPECULAR, "Specular"},
    {Material::SHININESS, "Shininess"},
    {Material::NORMAL, "Normal"},
    {Material::DISPLACEMENT, "Displacement"}};

const char *InteractiveScene::LIGHT_TYPE_LABEL[] = {"Directional", "Point", "Spotlight"};

char InteractiveScene::repository_url[] = "";

void InteractiveScene::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{

    Scene::framebufferSizeCallback(window, width, height);

    static_cast<InteractiveScene *>(glfwGetWindowUserPointer(window))->mouse->setResolution(width, height);
}

void InteractiveScene::mouseButtonCallback(GLFWwindow *window, int, int action, int)
{

    ImGuiIO &io = ImGui::GetIO();
    const bool capture_io = io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;

    if (!capture_io && (action == GLFW_RELEASE))
    {
        static_cast<InteractiveScene *>(glfwGetWindowUserPointer(window))->setCursorEnabled(false);
    }
}

void InteractiveScene::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{

    InteractiveScene *const scene = static_cast<InteractiveScene *>(glfwGetWindowUserPointer(window));
    scene->cursor_position.x = static_cast<float>(xpos);
    scene->cursor_position.y = static_cast<float>(ypos);

    ImGuiIO &io = ImGui::GetIO();
    const bool capture_io = io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;

    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouse) && (!capture_io || !scene->show_main_gui))
    {

        scene->active_camera->rotate(scene->mouse->translate(xpos, ypos));

        const glm::vec3 direction = scene->active_camera->getDirection();
        for (std::pair<const std::size_t, Light *> &light_data : scene->light_stock)
        {
            if (light_data.second->isGrabbed())
            {
                light_data.second->setDirection(direction);
            }
        }
    }
}

void InteractiveScene::scrollCallback(GLFWwindow *window, double, double yoffset)
{

    ImGuiIO &io = ImGui::GetIO();
    const bool capture_io = io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;

    if (!capture_io)
    {
        static_cast<InteractiveScene *>(glfwGetWindowUserPointer(window))->active_camera->zoom(yoffset);
    }
}

void InteractiveScene::keyCallback(GLFWwindow *window, int key, int, int action, int modifier)
{

    bool pressed = action != GLFW_RELEASE;

    InteractiveScene *const scene = static_cast<InteractiveScene *>(glfwGetWindowUserPointer(window));

    ImGuiIO &io = ImGui::GetIO();
    const bool capture_io = io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;

    switch (key)
    {

    case GLFW_KEY_ESCAPE:
        if (pressed)
        {
            scene->show_main_gui = !capture_io || !scene->show_main_gui;
            scene->setCursorEnabled(scene->show_main_gui);
        }
        return;

    case GLFW_KEY_F1:
        if (pressed)
        {
            scene->setAboutVisible(!scene->show_about);
        }
        return;

    case GLFW_KEY_F11:
        if (pressed)
        {
            scene->setAboutImGuiVisible(!scene->show_about_imgui);
        }
        return;

    case GLFW_KEY_F12:
        if (pressed)
        {
            scene->setMetricsVisible(!scene->show_metrics);
        }
        return;

    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        Camera::setBoosted(pressed);
        return;

    case GLFW_KEY_R:
        if (pressed && (modifier == GLFW_MOD_CONTROL))
        {
            scene->reloadPrograms();
        }
    }
}

void InteractiveScene::drawGUI()
{

    if (!show_main_gui && !show_metrics && !show_about && !show_about_imgui)
    {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_main_gui)
    {
        showMainGUIWindow();
    }

    if (show_about)
    {
        InteractiveScene::showAboutWindow(show_about);
    }

    if (show_metrics)
    {
        ImGui::ShowMetricsWindow(&show_metrics);
    }

    if (show_about_imgui)
    {
        ImGui::ShowAboutWindow(&show_about_imgui);
    }

    switch (focus)
    {

    case InteractiveScene::GUI:
        ImGui::SetWindowFocus();
        focus = InteractiveScene::NONE;
        break;

    case InteractiveScene::SCENE:
        ImGui::SetWindowFocus(nullptr);
        focus = InteractiveScene::NONE;
        break;

    case InteractiveScene::NONE:
        break;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void InteractiveScene::showMainGUIWindow()
{

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::SetNextWindowPos(ImVec2(0.0F, 0.0F), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(501.0F, static_cast<float>(height)), ImGuiCond_Always);

    const bool open = ImGui::Begin("Settings", &show_main_gui, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();

    if (!show_main_gui || !open)
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("User Guide", ImGuiTreeNodeFlags_DefaultOpen))
    {

        ImGui::BulletText("ESCAPE to toggle the navigation mode.");
        ImGui::BulletText("Click in the scene to enter in the navigation mode.");
        ImGui::BulletText("F1 to toggle the about window.");
        ImGui::BulletText("F12 to toggle the Dear ImGui metrics window.");
        ImGui::BulletText("Double-click on title bar to collapse window.");

        if (ImGui::TreeNodeEx("Others than the settings window", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("Click and drag on lower right corner to resize window\n(double-click to auto fit window to its contents).");
            ImGui::BulletText("Click and drag on any empty space to move window.");
            ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
            ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNodeEx("While editing text", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("Hold SHIFT or use mouse to select text.");
            ImGui::BulletText("CTRL+Left/Right to word jump.");
            ImGui::BulletText("CTRL+A or double-click to select all.");
            ImGui::BulletText("CTRL+X,CTRL+C,CTRL+V to use clipboard.");
            ImGui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
            ImGui::BulletText("ESCAPE to revert.");
            ImGui::BulletText("You can apply arithmetic operators +,*,/ on numerical\nvalues. Use +- to subtract.");
            ImGui::TreePop();
            ImGui::Spacing();
        }

        show_about |= ImGui::Button("About OBJViewer");

        ImGui::SameLine();
        show_about_imgui |= ImGui::Button("About Dear ImGui");

        ImGui::SameLine();
        show_metrics |= ImGui::Button("Metrics");
    }

    if (ImGui::CollapsingHeader("Scene"))
    {

        if (ImGui::TreeNodeEx("OpenGL", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Vendor: %s", Scene::opengl_vendor);
            ImGui::Text("Renderer: %s", Scene::opengl_renderer);
            ImGui::Text("Version: %s", Scene::opengl_version);
            ImGui::Text("GLSL version: %s", Scene::glsl_version);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Window", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Width:  %d", width);
            ImGui::SameLine(210.0F);
            ImGui::Text("Seconds: %.3fs", glfwGetTime());
            ImGui::Text("Height: %d", height);
            ImGui::SameLine(210.0F);
            ImGui::Text("Frames:  %.3fE3", kframes);
            ImGui::Text("Mouse: %.0f, %.0f", cursor_position.x, cursor_position.y);
            ImGui::HelpMarker("[x, y]");
            ImGui::ColorEdit3("Background", &background_color.r);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Statistics*"))
        {

            std::size_t vertices = 0U;
            std::size_t elements = 0U;
            std::size_t triangles = 0U;
            std::size_t materials = 0U;
            std::size_t textures = 0U;

            for (const std::pair<const std::size_t, const std::pair<const Model *const, const std::size_t>> &program_data : model_stock)
            {
                vertices += program_data.second.first->getNumberOfVertices();
                elements += program_data.second.first->getNumberOfElements();
                triangles += program_data.second.first->getNumberOfTriangles();
                materials += program_data.second.first->getNumberOfMaterials();
                textures += program_data.second.first->getNumberOfTextures();
            }

            std::size_t shaders = 0U;
            for (const std::pair<const std::size_t, std::pair<const GLSLProgram *const, const std::string>> &program_data : program_stock)
            {
                shaders += program_data.second.first->getNumberOfShaders();
            }

            ImGui::BulletText("Cameras: %lu", camera_stock.size());

            if (ImGui::TreeNodeEx("modelstats", ImGuiTreeNodeFlags_DefaultOpen, "Models: %lu", model_stock.size()))
            {
                ImGui::Text("Elements:  %lu", elements);
                ImGui::HelpMarker("Total of vertices");
                ImGui::SameLine(210.0F);
                ImGui::Text("Materials: %lu", materials);
                ImGui::Text("Vertices:  %lu", vertices);
                ImGui::HelpMarker("Unique vertices");
                ImGui::SameLine(210.0F);
                ImGui::Text("Textures:  %lu", textures);
                ImGui::Text("Triangles: %lu", triangles);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("programsstats", ImGuiTreeNodeFlags_DefaultOpen, "GLSL programs: %lu", program_stock.size()))
            {
                ImGui::Text("Shaders: %lu", shaders);
                ImGui::TreePop();
            }

            ImGui::Spacing();
            ImGui::TextDisabled("*Including the elements with errors.");

            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Cameras"))
    {

        if (ImGui::TreeNodeEx("Global", ImGuiTreeNodeFlags_DefaultOpen))
        {

            float value = Camera::getSensibility();
            if (ImGui::DragFloat("Sensibility", &value, 0.25F, 0.0F, 0.0F, "%.4f"))
            {
                Camera::setSensibility(value);
            }

            value = Camera::getSpeed();
            if (ImGui::DragFloat("Speed", &value, 0.005F, 0.0F, FLT_MAX, "%.4f"))
            {
                Camera::setSpeed(value);
            }

            value = Camera::getBoostedSpeed();
            if (ImGui::DragFloat("Boost speed", &value, 0.05F, 0.0F, FLT_MAX, "%.4f"))
            {
                Camera::setBoostedSpeed(value);
            }
            ImGui::HelpMarker("The boost speed is expected to be\ngreater than the normal speed.");
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Active", ImGuiTreeNodeFlags_DefaultOpen))
        {
            cameraWidget(active_camera);
            ImGui::TreePop();
        }

        std::size_t remove = 0U;

        for (const std::pair<const std::size_t, Camera *const> &camera_data : camera_stock)
        {

            const std::string id = std::to_string(camera_data.first);
            const std::string camera_title = "Camera " + id;

            if (ImGui::TreeNode(id.c_str(), camera_title.c_str()))
            {
                if (!cameraWidget(camera_data.second, camera_data.first))
                {
                    remove = camera_data.first;
                }
                ImGui::TreePop();
            }
        }

        if (remove != 0U)
        {
            removeCamera(remove);
        }

        ImGui::Spacing();
        if (ImGui::Button("Add camera", ImVec2(454.0F, 19.0F)))
        {
            addCamera();
        }
        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Models"))
    {

        std::size_t remove = 0U;

        for (std::pair<const std::size_t, std::pair<Model *, std::size_t>> &program_data : model_stock)
        {

            const std::string id = std::to_string(program_data.first);
            const std::string program_title = "Model " + id + ": " + program_data.second.first->getName();

            if (ImGui::TreeNode(id.c_str(), program_title.c_str()))
            {
                if (!modelWidget(program_data.second))
                {
                    remove = program_data.first;
                }
                ImGui::TreePop();
            }
        }

        if (remove != 0U)
        {
            removeModel(remove);
        }

        ImGui::Spacing();
        if (ImGui::Button("Add model", ImVec2(454.0F, 19.0F)))
        {
            addModel();
        }
        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Lights"))
    {

        std::size_t remove = 0U;

        for (const std::pair<const std::size_t, Light *const> &light_data : light_stock)
        {

            const std::string id = std::to_string(light_data.first);
            const std::string light_title = "Light " + id;

            if (ImGui::TreeNode(id.c_str(), light_title.c_str()))
            {
                if (!lightWidget(light_data.second))
                {
                    remove = light_data.first;
                }
                ImGui::TreePop();
            }
        }

        if (remove != 0U)
        {
            removeLight(remove);
        }

        ImGui::Spacing();
        if (ImGui::Button("Add light", ImVec2(454.0F, 19.0F)))
        {
            addLight();
        }
        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("GLSL Programs"))
    {

        std::size_t remove = 0U;

        ImGui::BulletText("Lighting pass program");
        ImGui::Indent();

        std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(lighting_program);
        std::string program_title = (result == program_stock.end() ? "NULL" : result->second.second);
        if ((lighting_program != 0U) && (lighting_program != 1U))
        {
            program_title.append(" (").append(std::to_string(lighting_program)).append(")");
        }

        ImGui::PushItemWidth(-1.0F);
        if (ImGui::BeginCombo("###lighting_pass_program", program_title.c_str()))
        {

            size_t new_program = lighting_program;
            for (const std::pair<const std::size_t, std::pair<const GLSLProgram *const, const std::string>> &program_data : program_stock)
            {
                if (programComboItem(lighting_program, program_data.first))
                {
                    new_program = program_data.first;
                }
            }
            lighting_program = new_program;
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::Unindent();

        for (std::pair<const std::size_t, std::pair<GLSLProgram *, std::string>> &program_data : program_stock)
        {

            const std::string id = std::to_string(program_data.first);

            switch (program_data.first)
            {

            case 0U:
                program_title = "Default geometry pass";
                break;

            case 1U:
                program_title = "Default lighting pass";
                break;

            default:
                program_title = "Program " + id + ": " + program_data.second.second;
            }

            if (ImGui::TreeNode(id.c_str(), program_title.c_str()))
            {
                if (!programWidget(program_data.second))
                {
                    remove = program_data.first;
                }
                ImGui::TreePop();
            }
        }

        if (remove != 0U)
        {
            removeProgram(remove);
        }

        ImGui::Spacing();
        if (ImGui::Button("Add GLSL program", ImVec2(454.0F, 19.0F)))
        {
            addProgram("Empty");
        }
        ImGui::Spacing();
    }

    ImGui::End();
}

bool InteractiveScene::cameraWidget(Camera *const camera, const std::size_t &id)
{

    bool keep = true;

    if (id != 0U)
    {

        bool active = camera == active_camera;
        if (ImGui::Checkbox("Active", &active))
        {
            active_camera = camera;
        }

        if (camera_stock.size() > 1U)
        {
            keep = !ImGui::RemoveButton();
        }
    }

    bool orthogonal = camera->isOrthogonal();
    if (ImGui::RadioButton("Perspective", !orthogonal))
    {
        camera->setOrthogonal(false);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthogonal", orthogonal))
    {
        camera->setOrthogonal(true);
    }
    ImGui::SameLine(338.0F);
    ImGui::Text("Projection");

    glm::vec3 value = camera->getPosition();
    if (ImGui::DragFloat3("Position", &value.x, 0.01F, 0.0F, 0.0F, "%.4f"))
    {
        camera->setPosition(value);
    }

    value = camera->getDirection();
    if (ImGui::DragFloat3("Direction", &value.x, 0.01F, 0.0F, 0.0F, "%.4f"))
    {
        camera->setDirection(value);
    }

    glm::vec2 clipping = camera->getClipping();
    if (ImGui::DragFloat2("Clipping", &clipping.x, 0.01F, 0.0F, 0.0F, "%.4f"))
    {
        camera->setClipping(clipping);
    }
    ImGui::HelpMarker("(Near, Far)");

    float fov = camera->getFOV();
    if (ImGui::DragFloat("FOV", &fov, 0.01F, 0.0F, 0.0F, "%.4f"))
    {
        camera->setFOV(fov);
    }

    ImGui::Separator();

    return keep;
}

bool InteractiveScene::modelWidget(std::pair<Model *, std::size_t> &model_data)
{

    bool keep = true;

    Model *const model = model_data.first;
    const std::size_t program = model_data.second;

    std::string str = model->getPath();
    if (ImGui::InputText("Path", &str, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        model->setPath(str);
    }

    bool enabled = model->isEnabled() && model->isOpen();
    if (ImGui::Checkbox("Enabled", &enabled))
    {
        model->setEnabled(enabled && model->isOpen());
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload model"))
    {
        model->reload();
    }

    keep = !ImGui::RemoveButton();

    if (!model->isOpen())
    {
        if (!model->Model::getPath().empty())
        {
            ImGui::TextColored(ImVec4(0.80F, 0.16F, 0.16F, 1.00F), "Could not open the model");
        }
        return keep;
    }

    if (ImGui::TreeNodeEx("Sumary", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Elements:  %lu", model->getNumberOfElements());
        ImGui::HelpMarker("Total of vertices");
        ImGui::SameLine(210.0F);
        ImGui::Text("Materials: %lu", model->getNumberOfMaterials());
        ImGui::Text("Vertices:  %lu", model->getNumberOfVertices());
        ImGui::HelpMarker("Unique vertices");
        ImGui::SameLine(210.0F);
        ImGui::Text("Textures:  %lu", model->getNumberOfTextures());
        ImGui::Text("Triangles: %lu", model->getNumberOfTriangles());
        ImGui::TreePop();
    }

    std::map<std::size_t, std::pair<GLSLProgram *, std::string>>::const_iterator result = program_stock.find(program);
    std::string program_title = (result == program_stock.end() ? "NULL" : result->second.second);
    if ((program != 0U) && (program != 1U))
    {
        program_title.append(" (").append(std::to_string(program)).append(")");
    }

    if (ImGui::BeginCombo("GLSL program", program_title.c_str()))
    {

        size_t new_program = model_data.second;
        for (const std::pair<const std::size_t, std::pair<const GLSLProgram *const, const std::string>> &program_data : program_stock)
        {
            if (programComboItem(model_data.second, program_data.first))
            {
                new_program = program_data.first;
            }
        }
        model_data.second = new_program;
        ImGui::EndCombo();
    }

    if (ImGui::TreeNodeEx("Geometry", ImGuiTreeNodeFlags_DefaultOpen))
    {

        glm::vec3 value = model->Model::getPosition();
        if (ImGui::DragFloat3("Position", &value.x, 0.01F, 0.0F, 0.0F, "%.4f"))
        {
            model->Model::setPosition(value);
        }

        value = model->Model::getRotationAngles();
        if (ImGui::DragFloat3("Rotation", &value.x, 0.50F, 0.0F, 0.0F, "%.4f"))
        {
            model->Model::setRotation(value);
        }
        ImGui::HelpMarker("Angles in degrees");

        value = model->Model::getScale();
        if (ImGui::DragFloat3("Scale", &value.x, 0.01F, 0.0F, 0.0F, "%.4f"))
        {
            model->setScale(value);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Materials"))
    {

        str = model->getMaterialPath();
        ImGui::InputText("Path", &str, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
        ImGui::HelpMarker("Read only");

        if (!str.empty())
        {
            if (ImGui::Button("Reload material"))
            {
                model->reloadMaterial();
            }
            ImGui::HelpMarker("If the material data has\nchanged, nothing happens");
        }

        if (!model->isMaterialOpen())
        {
            ImGui::TextColored(ImVec4(0.80F, 0.16F, 0.16F, 1.00F), "Could not open the material file");
        }

        float value;
        glm::vec3 color;
        Material *material;
        Material::Attribute material_attribute;
        const std::size_t materials = model->getNumberOfMaterials();

        if (ImGui::TreeNodeEx("Global", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextDisabled("Chages will be applied to all materials");

            material = model->getDefaultMaterial();

            material_attribute = Material::AMBIENT;
            color = material->getColor(material_attribute);
            if (ImGui::ColorEdit3("Ambient", &color.x))
            {
                material->setColor(material_attribute, color);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setColor(material_attribute, color);
                }
            }

            material_attribute = Material::DIFFUSE;
            color = material->getColor(material_attribute);
            if (ImGui::ColorEdit3("Diffuse", &color.x))
            {
                material->setColor(material_attribute, color);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setColor(material_attribute, color);
                }
            }

            material_attribute = Material::SPECULAR;
            color = material->getColor(material_attribute);
            if (ImGui::ColorEdit3("Specular", &color.x))
            {
                material->setColor(material_attribute, color);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setColor(material_attribute, color);
                }
            }

            material_attribute = Material::SHININESS;
            value = material->getValue(material_attribute);
            if (ImGui::DragFloat("Shininess", &value, 0.01F, 0.0F, 1000.0F, "%.4f"))
            {
                material->setValue(material_attribute, value);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setValue(material_attribute, value);
                }
            }

            material_attribute = Material::ROUGHNESS;
            value = material->getValue(material_attribute);
            if (ImGui::DragFloat("Roughness", &value, 0.005F, 0.0F, 1.0F, "%.4f"))
            {
                material->setValue(material_attribute, value);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setValue(material_attribute, value);
                }
            }

            material_attribute = Material::METALNESS;
            value = material->getValue(material_attribute);
            if (ImGui::DragFloat("Metalness", &value, 0.005F, 0.0F, 1.0F, "%.4f"))
            {
                material->setValue(material_attribute, value);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setValue(material_attribute, value);
                }
            }

            material_attribute = Material::DISPLACEMENT;
            value = material->getValue(material_attribute);
            if (ImGui::DragFloat("Parallax", &value, 0.005F, 0.0F, 0.0F, "%.4f"))
            {
                material->setValue(material_attribute, value);
                for (std::size_t i = 0U; i < materials; i++)
                {
                    model->getMaterial(i)->setValue(material_attribute, value);
                }
            }

            if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextDisabled("Chages will be applied to all textures");

                material_attribute = Material::AMBIENT;
                enabled = material->isTextureEnabled(material_attribute);
                if (ImGui::Checkbox("Ambient", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                material_attribute = Material::SHININESS;
                enabled = material->isTextureEnabled(material_attribute);
                ImGui::SameLine(210.0F);
                if (ImGui::Checkbox("Shininess", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                material_attribute = Material::DIFFUSE;
                enabled = material->isTextureEnabled(material_attribute);
                if (ImGui::Checkbox("Diffuse", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                material_attribute = Material::NORMAL;
                enabled = material->isTextureEnabled(material_attribute);
                ImGui::SameLine(210.0F);
                if (ImGui::Checkbox("Normal", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                material_attribute = Material::SPECULAR;
                enabled = material->isTextureEnabled(material_attribute);
                if (ImGui::Checkbox("Specular", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                material_attribute = Material::DISPLACEMENT;
                enabled = material->isTextureEnabled(material_attribute);
                ImGui::SameLine(210.0F);
                if (ImGui::Checkbox("Displacement", &enabled))
                {
                    material->setTextureEnabled(material_attribute, enabled);
                    for (std::size_t i = 0U; i < materials; i++)
                    {
                        model->getMaterial(i)->setTextureEnabled(material_attribute, enabled);
                    }
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        for (std::size_t i = 0; i < materials; i++)
        {

            material = model->getMaterial(i);
            std::string name = material->getName();
            std::string id = std::to_string(i);

            if (ImGui::TreeNode(id.c_str(), name.c_str()))
            {

                if (ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    material->setName(name);
                }

                material_attribute = Material::AMBIENT;
                color = material->getColor(material_attribute);
                if (ImGui::ColorEdit3("Ambient", &color.x))
                {
                    material->setColor(material_attribute, color);
                }

                material_attribute = Material::DIFFUSE;
                color = material->getColor(material_attribute);
                if (ImGui::ColorEdit3("Diffuse", &color.x))
                {
                    material->setColor(material_attribute, color);
                }

                material_attribute = Material::SPECULAR;
                color = material->getColor(material_attribute);
                if (ImGui::ColorEdit3("Specular", &color.x))
                {
                    material->setColor(material_attribute, color);
                }

                material_attribute = Material::SHININESS;
                value = material->getValue(material_attribute);
                if (ImGui::DragFloat("Shininess", &value, 0.01F, 0.0F, 1000.0F, "%.4f"))
                {
                    material->setValue(material_attribute, value);
                }

                material_attribute = Material::ROUGHNESS;
                value = material->getValue(material_attribute);
                if (ImGui::DragFloat("Roughness", &value, 0.005F, 0.0F, 1.0F, "%.4f"))
                {
                    material->setValue(material_attribute, value);
                }

                material_attribute = Material::METALNESS;
                value = material->getValue(material_attribute);
                if (ImGui::DragFloat("Metalness", &value, 0.005F, 0.0F, 1.0F, "%.4f"))
                {
                    material->setValue(material_attribute, value);
                }

                material_attribute = Material::DISPLACEMENT;
                value = material->getValue(material_attribute);
                if (ImGui::DragFloat("Parallax", &value, 0.005F, 0.0F, 0.0F, "%.4f"))
                {
                    material->setValue(material_attribute, value);
                }

                if (ImGui::TreeNode("Textures"))
                {

                    for (const std::pair<const Material::Attribute, const std::string> &texture : InteractiveScene::AVAILABLE_TEXTURE)
                    {

                        id = std::to_string(texture.first);
                        if (ImGui::TreeNode(id.c_str(), texture.second.c_str()))
                        {

                            str = material->getTexturePath(texture.first);
                            if (ImGui::InputText("Path", &str, ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                material->setTexturePath(texture.first, str);
                            }

                            enabled = material->isTextureEnabled(texture.first);
                            if (ImGui::Checkbox("Enabled", &enabled))
                            {
                                material->setTextureEnabled(texture.first, enabled);
                            }

                            ImGui::SameLine();
                            if (ImGui::Button("Reload texture"))
                            {
                                material->reloadTexture(texture.first);
                            }

                            ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(material->getTexture(texture.first))), ImVec2(300.0F, 300.0F), ImVec2(0.0F, 1.0F), ImVec2(1.0F, 0.0F));

                            ImGui::Separator();

                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::Separator();

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    ImGui::Separator();

    return keep;
}

bool InteractiveScene::lightWidget(Light *const light)
{

    bool keep = true;

    Light::Type type = light->getType();
    if (ImGui::BeginCombo("Type", InteractiveScene::LIGHT_TYPE_LABEL[type]))
    {
        for (GLint i = Light::DIRECTIONAL; i <= Light::SPOTLIGHT; i++)
        {

            const Light::Type new_type = static_cast<Light::Type>(i);
            bool selected = type == new_type;

            if (ImGui::Selectable(InteractiveScene::LIGHT_TYPE_LABEL[new_type], selected))
            {
                light->setType(new_type);
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    bool status = light->isEnabled();
    if (ImGui::Checkbox("Enabled", &status))
    {
        light->setEnabled(status);
    }

    if (type == Light::SPOTLIGHT)
    {
        status = light->isGrabbed();
        ImGui::SameLine();
        if (ImGui::Checkbox("Grabbed", &status))
        {
            light->setGrabbed(status);
        }
    }

    if (light_stock.size() > 1U)
    {
        keep = !ImGui::RemoveButton();
    }

    ImGui::BulletText("Spacial attributes");
    ImGui::Indent();

    if (type != Light::POINT)
    {
        glm::vec3 direction = light->getDirection();
        if (ImGui::DragFloat3("Direction", &direction.x, 0.01F, 0.0F, 0.0F, "%.4f"))
        {
            light->setDirection(direction);
        }
    }

    if (type != Light::DIRECTIONAL)
    {

        glm::vec3 vector = light->getPosition();
        if (ImGui::DragFloat3("Position", &vector.x, 0.01F, 0.0F, 0.0F, "%.4f"))
        {
            light->setPosition(vector);
        }

        vector = light->getAttenuation();
        if (ImGui::DragFloat3("Attenuation", &vector.x, 0.01F, 0.0F, 0.0F, "%.4f"))
        {
            light->setAttenuation(vector);
        }
        ImGui::HelpMarker("[Constant, Linear, Quadratic]\nIf any value is negative rare\neffects may appear.");

        if (type == Light::SPOTLIGHT)
        {
            glm::vec2 cutoff = light->getCutoff();
            if (ImGui::DragFloat2("Cutoff", &cutoff.x, 0.01F, 0.0F, 0.0F, "%.4f"))
            {
                light->setCutoff(cutoff);
            }
            ImGui::HelpMarker("[Inner, Outter]\nIf the inner cutoff is greater than the\noutter cutoff rare effects may appear.");
        }
    }
    ImGui::Unindent();

    ImGui::BulletText("Color attributes");
    ImGui::Indent();

    glm::vec3 color = light->getAmbientColor();
    if (ImGui::ColorEdit3("Ambient", &color.r))
    {
        light->setAmbientColor(color);
    }

    color = light->getDiffuseColor();
    if (ImGui::ColorEdit3("Diffuse", &color.r))
    {
        light->setDiffuseColor(color);
    }

    color = light->getSpecularColor();
    if (ImGui::ColorEdit3("Specular", &color.r))
    {
        light->setSpecularColor(color);
    }
    ImGui::Unindent();

    ImGui::BulletText("Color values");
    ImGui::Indent();

    float value = light->getAmbientLevel();
    if (ImGui::DragFloat("Ambient level", &value, 0.0025F, 0.0F, 1.0F, "%.4f"))
    {
        light->setAmbientLevel(value);
    }

    value = light->getDiffuseLevel();
    if (ImGui::DragFloat("Diffuse level", &value, 0.0025F, 0.0F, 1.0F, "%.4f"))
    {
        light->setDiffuseLevel(value);
    }

    value = light->getSpecularLevel();
    if (ImGui::DragFloat("Specular level", &value, 0.0025F, 0.0F, 1.0F, "%.4f"))
    {
        light->setSpecularLevel(value);
    }

    value = light->getShininess();
    if (ImGui::DragFloat("Shininess", &value, 0.0025F, 0.0F, 0.0F, "%.4f"))
    {
        light->setShininess(value);
    }
    ImGui::HelpMarker("If the shininess value negative\nrare effects may appear.");
    ImGui::Unindent();

    return keep;
}

bool InteractiveScene::programWidget(std::pair<GLSLProgram *, std::string> &program_data)
{

    bool keep = true;

    GLSLProgram *const program = program_data.first;
    std::string vert = program->getShaderPath(GL_VERTEX_SHADER);
    std::string geom = program->getShaderPath(GL_GEOMETRY_SHADER);
    std::string frag = program->getShaderPath(GL_FRAGMENT_SHADER);

    const bool default_program = (program == program_stock[0U].first) || (program == program_stock[1U].first);

    std::string description = program_data.second;
    if (ImGui::InputText("Description", &description, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        program_data.second = description;
    }

    if (ImGui::Button("Reload"))
    {
        program->link();
    }

    if (!default_program)
    {
        keep = !ImGui::RemoveButton();
    }

    if (!program->isValid())
    {
        ImGui::TextColored(ImVec4(0.80F, 0.16F, 0.16F, 1.00F), "Could not link the program");
    }

    ImGui::BulletText("Shaders");

    bool link = ImGui::InputText("Vertex", &vert, ImGuiInputTextFlags_EnterReturnsTrue);

    if (!default_program)
    {
        link |= ImGui::InputText("Geometry", &geom, ImGuiInputTextFlags_EnterReturnsTrue);
    }

    link |= ImGui::InputText("Fragment", &frag, ImGuiInputTextFlags_EnterReturnsTrue);

    if (link)
    {

        if (geom.empty())
        {
            program->link(vert, frag);
        }

        else
        {
            program->link(vert, geom, frag);
        }
    }

    ImGui::Separator();

    return keep;
}

bool InteractiveScene::programComboItem(const std::size_t &current, const std::size_t &program)
{

    bool selected = current == program;

    std::string program_title = program_stock[program].second;
    if ((program != 0U) && (program != 1U))
    {
        program_title.append(" (").append(std::to_string(program)).append(")");
    }

    const bool selection = ImGui::Selectable(program_title.c_str(), selected);

    if (selected)
    {
        ImGui::SetItemDefaultFocus();
    }

    return selection;
}

void InteractiveScene::processKeyboardInput()
{

    ImGuiIO &io = ImGui::GetIO();
    if ((io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput) && show_main_gui)
    {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_W))
        active_camera->travell(Camera::FRONT);
    if (glfwGetKey(window, GLFW_KEY_S))
        active_camera->travell(Camera::BACK);
    if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
        active_camera->travell(Camera::LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
        active_camera->travell(Camera::RIGHT);
    if (glfwGetKey(window, GLFW_KEY_SPACE) || glfwGetKey(window, GLFW_KEY_UP))
        active_camera->travell(Camera::UP);
    if (glfwGetKey(window, GLFW_KEY_C) || glfwGetKey(window, GLFW_KEY_DOWN))
        active_camera->travell(Camera::DOWN);

    const glm::vec3 position = active_camera->getPosition();
    for (std::pair<const std::size_t, Light *> &light_data : light_stock)
    {
        if (light_data.second->isGrabbed())
        {
            light_data.second->setPosition(position);
        }
    }
}

InteractiveScene::InteractiveScene(const std::string &title, const int &width, const int &height, const int &context_ver_maj, const int &context_ver_min) :

                                                                                                                                                            Scene(title, width, height, context_ver_maj, context_ver_min),

                                                                                                                                                            mouse(new Mouse(width, height)),

                                                                                                                                                            cursor_enabled(true),
                                                                                                                                                            cursor_position(0.0F),

                                                                                                                                                            focus(InteractiveScene::GUI),

                                                                                                                                                            show_main_gui(true),
                                                                                                                                                            show_metrics(false),
                                                                                                                                                            show_about(false),
                                                                                                                                                            show_about_imgui(false),

                                                                                                                                                            focus_gui(true)
{

    if ((Scene::instances == 1U) && Scene::initialized_glad)
    {

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, InteractiveScene::framebufferSizeCallback);
        glfwSetMouseButtonCallback(window, InteractiveScene::mouseButtonCallback);
        glfwSetCursorPosCallback(window, InteractiveScene::cursorPosCallback);
        glfwSetScrollCallback(window, InteractiveScene::scrollCallback);
        glfwSetKeyCallback(window, InteractiveScene::keyCallback);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = NULL;

        ImGui::StyleColorsDark();
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0F);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 2.0F);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool InteractiveScene::isMainGUIVisible() const
{
    return show_main_gui;
}

bool InteractiveScene::isMetricsVisible() const
{
    return show_metrics;
}

bool InteractiveScene::isAboutVisible() const
{
    return show_about;
}

bool InteractiveScene::isAboutImGuiVisible() const
{
    return show_about_imgui;
}

bool InteractiveScene::isCursorEnabled() const
{
    return ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse;
}

Mouse *InteractiveScene::getMouse() const
{
    return mouse;
}

void InteractiveScene::setMainGUIVisible(const bool &status)
{
    show_main_gui = status;
    focus = InteractiveScene::GUI;
}

void InteractiveScene::setMetricsVisible(const bool &status)
{

    show_metrics = status;

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse)
    {
        focus = InteractiveScene::SCENE;
    }
}

void InteractiveScene::setAboutVisible(const bool &status)
{

    show_about = status;

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse)
    {
        focus = InteractiveScene::SCENE;
    }
}

void InteractiveScene::setAboutImGuiVisible(const bool &status)
{

    show_about_imgui = status;

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse)
    {
        focus = InteractiveScene::SCENE;
    }
}

void InteractiveScene::setCursorEnabled(const bool &status)
{

    if (status)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        focus = InteractiveScene::GUI;
    }

    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        focus = InteractiveScene::SCENE;

        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouse->setTranslationPoint(xpos, ypos);
    }
}

/** Render main loop */
void InteractiveScene::mainLoop()
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawScene();
        drawGUI();

        processKeyboardInput();

        glfwPollEvents();
        glfwSwapBuffers(window);

        kframes += 0.001;
    }
}

InteractiveScene::~InteractiveScene()
{

    delete mouse;

    if ((Scene::instances == 1U) && Scene::initialized_glad)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void InteractiveScene::showAboutWindow(bool &show)
{

    if (!ImGui::Begin("About OBJViewer", &show, ImGuiWindowFlags_NoResize))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("OBJViewer - Another OBJ models viewer");
    ImGui::Separator();

    ImGui::Text("By Erick Rincones 2019.");
    ImGui::Text("OBJViewer is licensed under the MIT License, see LICENSE for more information.");
    ImGui::Spacing();

    ImGui::Text("GitHub repository:");
    ImGui::HelpMarker("Click to select all and press\nCTRL+V to copy to clipboard");

    ImGui::PushItemWidth(-1.0F);
    ImGui::InputText("###repourl", InteractiveScene::repository_url, sizeof(InteractiveScene::repository_url), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    ImGui::End();
}