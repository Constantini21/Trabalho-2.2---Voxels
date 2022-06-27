#include "scene/gui/interactivescene.hpp"
#include "dirsep.h"
#include <glm/vec3.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }

    InteractiveScene *scene = new InteractiveScene("Obj Loader Gustavo Constantini, Davi Mello e Douglas da Rocha");

    if (!scene->isValid())
    {
        delete scene;
        return 1;
    }

    // background color
    scene->setBackgroundColor(glm::vec3(0.85F, 0.75F, 0.80F));

    const std::string binPath = argv[0];
    const std::string relative = binPath.substr(0U, binPath.find_last_of(DIR_SEP) + 1U);

    const std::string modelPath = relative + ".." + DIR_SEP + "model" + DIR_SEP;
    const std::string shaderPath = relative + ".." + DIR_SEP + "shader" + DIR_SEP;

    const std::string commonLpPath = shaderPath + "lp_common.vert.glsl";
    scene->setDefaultGeometryPassProgram("[GP] Basic shading", shaderPath + "gp_basic.vert.glsl", shaderPath + "gp_basic.frag.glsl");
    scene->setDefaultLightingPassProgram("[LP] Normals", commonLpPath, shaderPath + "lp_normals.frag.glsl");

    const std::string gpNormalVertPath = shaderPath + "gp_normal.vert.glsl";
    std::size_t normal = scene->addProgram("[GP] Normal mapping", gpNormalVertPath, shaderPath + "gp_normal.frag.glsl");
    std::size_t parallax = scene->addProgram("[GP] Parallax mapping", gpNormalVertPath, shaderPath + "gp_parallax.frag.glsl");

    scene->addProgram("[LP] Positions", commonLpPath, shaderPath + "lp_positions.frag.glsl");

    scene->addProgram("[LP] Blinn-Phong", commonLpPath, shaderPath + "lp_blinn_phong.frag.glsl");
    scene->addProgram("[LP] Oren-Nayar", commonLpPath, shaderPath + "lp_oren_nayar.frag.glsl");
    std::size_t lpProgram = scene->addProgram("[LP] Cock-Torrance", commonLpPath, shaderPath + "lp_cock_torrance.frag.glsl");

    scene->setLightingPassProgram(lpProgram);

    std::size_t modelId0 = scene->addModel(modelPath + "cat" + DIR_SEP + "12221_Cat_v1_l3.obj", normal);

    Model *model = scene->getModel(modelId0);

    model = scene->getModel(modelId0);
    model->setScale(glm::vec3(0.45F));
    model->setPosition(glm::vec3(0.6F, -0.225F, 0.0F));

    // main loop
    scene->mainLoop();

    delete scene;
    return 0;
}