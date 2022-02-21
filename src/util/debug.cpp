#include <filesystem>

#include <renderer/global.h>
#include <renderer/shader.h>

#include "util.h"

namespace util::debug {
    fs::path vert = global::resolvePath("src/shader/debug/shader.vert");
    fs::path frag = global::resolvePath("src/shader/debug/shader.frag");
    Shader shader(vert.c_str(), frag.c_str());

    GLuint planeVAO = 0, planeVBO = 0;

    float planeVertices[] = {
        // NDC         // UV
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
};

using namespace util;
using namespace util::debug;

void Debug::init() {
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

/**
 * 贴图渲染
 * @brief 将贴图渲染并铺满至整个屏幕
 * @param texture 需要渲染的贴图
 */ 
void Debug::render(GLuint texture) {
    if (planeVAO == 0)
        init();

    glDisable(GL_DEPTH_TEST);

    debug::shader.use();
    debug::shader.setInt("singleColor", singleColor);
    debug::shader.setInt("theTexture", 0);

    glBindVertexArray(debug::planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
