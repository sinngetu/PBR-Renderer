#include <filesystem>

#include <renderer/global.h>
#include <renderer/shader.h>
#include <renderer/material.h>

#include "material.h"
#include "../util/util.h"

namespace material::pbr {
    fs::path vert = global::resolvePath("src/shader/PBR/shader.vert");
    fs::path frag = global::resolvePath("src/shader/PBR/shader.frag");

    Shader shader(vert.c_str(), frag.c_str());
}

using namespace material;
using namespace glm;

PBR::PBR():Material(pbr::shader) {};

void PBR::setup(
    GLuint envMap,
    const char* baseColor,
    const char* metallic,
    const char* roughness,
    const char* normal,
    const char* height,
    const char* ao
) {
    baseColorMap = global::loadTexture(baseColor);
    metallicMap = global::loadTexture(metallic);
    roughnessMap = global::loadTexture(roughness);
    normalMap = global::loadTexture(normal);
    heightMap = global::loadTexture(height);
    aoMap = global::loadTexture(ao);

    GLuint gaussianFilterMap = envMap;
    for(unsigned int i = 0; i < 10; i++)
        gaussianFilterMap = util::gaussianFilterCubemap(gaussianFilterMap);

    GLuint bilateralFilterMap = util::bilateralFilterCubemap(envMap);
    GLuint HDRMap = util::inverseToneMapping(envMap, gaussianFilterMap, bilateralFilterMap);

    irradianceMap = util::generateIrradianceMap(HDRMap);
    prefilterMap = util::generatePrefilterMap(HDRMap);
    brdfLUT = util::generateBRDFLUT();

    use();
    shader->setInt("material.baseColorMap", 0);
    shader->setInt("material.metallicMap", 1);
    shader->setInt("material.roughnessMap", 2);
    shader->setInt("material.normalMap", 3);
    shader->setInt("material.heightMap", 4);
    shader->setInt("material.aoMap", 5);
    shader->setInt("irradianceMap", 6);
    shader->setInt("prefilterMap", 7);
    shader->setInt("brdfLUT", 8);
    shader->setInt("shadowMap", 9);
}

void PBR::setTextures() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, metallicMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roughnessMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, aoMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
}

void PBR::setHeightScale(float scale) {
    heightScale = scale;
}

void PBR::setLight(vec3 direction, vec3 color) {
    use();
    shader->setVec3("light.direction", direction);
    shader->setVec3("light.color", color);
}

void PBR::setShadow(GLuint shadowMap, mat4 WorldToLight) {
    use();
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    shader->setMat4("WorldToLight", WorldToLight);
}
