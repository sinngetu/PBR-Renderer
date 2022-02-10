#include <filesystem>

#include <renderer/global.h>
#include <renderer/shader.h>
#include <renderer/material.h>
#include <renderer/model.h>

#include "material.h"

namespace material::shadow {
    fs::path vert = global::resolvePath("src/shader/shadow/shader.vert");
    fs::path frag = global::resolvePath("src/shader/shadow/shader.frag");
}

using namespace material;
using namespace glm;

Shadow::Shadow():shadowShader(shadow::vert.c_str(), shadow::frag.c_str()) {}

void Shadow::init() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);

    // init depth map
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, global::SHADOW_WIDTH, global::SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // border sampling handle
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // init FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Shadow::setShadowPorps(vec3 direction, vec3 offset, GLfloat halfWidth, GLfloat near, GLfloat far) {
    if(!inited) {
        init();
        inited = true;
    }

    this->direction = direction;
    this->offset = offset;
    this->halfWidth = halfWidth;
    this->near = near;
    this->far = far;
}

void Shadow::shadowMapping(std::vector<Model> models) {
    mat4 projection = ortho(-halfWidth, halfWidth, -halfWidth, halfWidth, near, far);
    mat4 view = lookAt(direction + offset, offset, vec3(0.0, 1.0, 0.0));
    WorldToLight = projection * view;

    shadowShader.use();
    shadowShader.setMat4("WorldToLight", WorldToLight);

    glViewport(0, 0, global::SHADOW_WIDTH, global::SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    for(unsigned int i = 0; i < models.size(); i++) {
        shadowShader.setMat4("model", models[i].getM());
        models[i].Draw();
    }

    // reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
}

GLuint Shadow::getShadowMap() {
    return depthMap;
}

void Shadow::setShadowTextureIndex(unsigned int i) {
    textureIndex = i;
}

void Shadow::setShadowMap() {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

mat4 Shadow::getWorldToLight() {
    return WorldToLight;
}
