#include <glad/glad.h>

#include <renderer/shader.h>
#include <renderer/global.h>

#include "util.h"

using namespace glm;

namespace util::convolution {
    mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
    mat4 views[] = {
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
        lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
    };
}

using namespace util::convolution;

namespace util::convolution::irradiance {
    fs::path vert = global::resolvePath("src/shader/convolution/shader.vert");
    fs::path frag = global::resolvePath("src/shader/convolution/irradiance.frag");
    fs::path geom = global::resolvePath("src/shader/convolution/shader.geom");
    Shader shader(vert.c_str(), frag.c_str(), geom.c_str());
}

/**
 * 生成辐照度贴图
 * @param cubemap    环境贴图
 * @param resolution 生成的立方体贴图的每一面的分辨率
 * @return 辐照度贴图
 */
GLuint util::generateIrradianceMap(GLuint &cubemap, unsigned int resolution) {
    GLuint FBO, irradianceMap;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &irradianceMap);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, irradianceMap, 0);

    irradiance::shader.use();
    irradiance::shader.setInt("env", 0);
    irradiance::shader.setMat4("projection", projection);

    for (unsigned int i = 0; i < 6; i++)
        irradiance::shader.setMat4("views[" + std::to_string(i) + "]", views[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    global::drawCube();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return irradianceMap;
}


namespace util::convolution::prefilter {
    fs::path vert = global::resolvePath("src/shader/convolution/shader.vert");
    fs::path frag = global::resolvePath("src/shader/convolution/prefilter.frag");
    fs::path geom = global::resolvePath("src/shader/convolution/shader.geom");
    Shader shader(vert.c_str(), frag.c_str(), geom.c_str());
}

/**
 * 生成预过滤贴图
 * @param cubemap    环境贴图
 * @param resolution 生成的立方体贴图的每一面的分辨率
 * @return 预过滤贴图
 */
GLuint util::generatePrefilterMap(GLuint &cubemap, unsigned int resolution) {
    GLuint FBO, prefilterMap;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &prefilterMap);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilter::shader.use();
    prefilter::shader.setInt("env", 0);
    prefilter::shader.setMat4("projection", projection);

    for (unsigned int i = 0; i < 6; i++)
        prefilter::shader.setMat4("views[" + std::to_string(i) + "]", views[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; mip++) {
        // reisze framebuffer according to mip-level size
        unsigned int mipWidth  = static_cast<unsigned int>(resolution * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(resolution * std::pow(0.5, mip));
        float roughness = (float)mip / (float)(maxMipLevels - 1);

        // glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, prefilterMap, mip);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        prefilter::shader.setFloat("roughness", roughness);

        global::drawCube();
    }

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return prefilterMap;
}
