#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <renderer/global.h>
#include <renderer/shader.h>

#include "scene/scene.h"
#include "util/util.h"

using namespace glm;
using namespace global;

int main() {
    if (!init()) return -1;

    auto envPath = global::resolvePath("assets/skybox/3.png");
    auto envMap = global::loadCubemap(envPath.c_str());

    // BRDF LUT
    // util::generateBRDFLUT(512, FBO);
    // util::write::image(FBO, "BRDF_LUT.png", 512, 512);


    /*== TODO: 完善 HDR 图片存储，使用 HDR 计算 =========================*/
    GLuint HDRMap = envMap;

    // GLuint gaussianFilterMap = envMap;
    // for(unsigned int i = 0; i < 10; i++)
    //     gaussianFilterMap = util::gaussianFilterCubemap(gaussianFilterMap);

    // GLuint bilateralFilterMap = util::bilateralFilterCubemap(envMap);
    // GLuint HDRMap = util::inverseToneMapping(envMap, gaussianFilterMap, bilateralFilterMap, 1024);
    /*==============================================================*/

    // irradiance
    auto irradianceMap = util::generateIrradianceMap(HDRMap, 32, FBO);
    util::write::cubemap(irradianceMap, FBO, "irradiance.png", 32);


    /*== TODO: 完善 minimap 存储, 改为保存 prefilterMap ================*/
    // prefilter env
    // auto prefilterMap = util::generatePrefilterMap(HDRMap, 128, FBO);
    // util::write::cubemap(HDRMap, FBO, "prefilter.png", 128);
    /*===============================================================*/

    glfwTerminate();
    return 0;
}
