#include <filesystem>

#include <renderer/global.h>
#include <renderer/shader.h>
#include <renderer/material.h>

#include "material.h"

namespace material::_default {
    fs::path vert = global::resolvePath("src/shader/default/shader.vert");
    fs::path frag = global::resolvePath("src/shader/default/shader.frag");

    Shader shader(vert.c_str(), frag.c_str());
}

using namespace material;
using namespace glm;

Default::Default():Material(material::_default::shader) {};

void Default::init() {
    use();
    shader->setInt("shadowMap", 0);
    setShadowTextureIndex(0);
}

void Default::setLight(vec3 direction, vec3 color) {
    use();
    shader->setVec3("light.direction", direction);
    shader->setVec3("light.color", color);
}

void Default::setShadow() {
    use();
    shader->setMat4("WorldToLight", WorldToLight);
    setShadowMap();
}
