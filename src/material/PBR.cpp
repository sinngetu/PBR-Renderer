#include <glad/glad.h>
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

PBR::PBR():Material(pbr::shader) {}

void PBR::setup() {
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
