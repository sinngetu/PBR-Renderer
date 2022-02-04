#include <filesystem>

#include <renderer/global.h>
#include <renderer/shader.h>
#include <renderer/material.h>

#include "material.h"

namespace material {
    namespace _defaultProps {
        fs::path vert = global::resolvePath("src/shader/default/shader.vert");
        fs::path frag = global::resolvePath("src/shader/default/shader.frag");

        unsigned int i = 0;
        Shader shader(vert.c_str(), frag.c_str());

        void setProps(Shader *shader) {
            
        };
    }
};

using namespace material;
using namespace glm;

Default::Default():Material(_defaultProps::shader, _defaultProps::setProps) {};

void Default::setMVP(mat4 M, mat4 V, mat4 P) {
    shader->setMat4("model", M);
    shader->setMat4("view", V);
    shader->setMat4("projection", P);
}

void Default::setCameraPositon(vec3 pos) {
    shader->setVec3("viewPosition", pos);
}

void Default::setLight(vec3 direction, vec3 color) {
    shader->setVec3("light.direction", direction);
    shader->setVec3("light.color", color);
}
