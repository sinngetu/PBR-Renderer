#include <glad/glad.h>

#include <renderer/shader.h>
#include <renderer/global.h>

#include "util.h"

using namespace glm;

// TODO: 抽离可复用的2D贴图与立方体贴图生成代码
namespace util::convolution {
    fs::path vert = global::resolvePath("src/shader/convolution/shader.vert");
    fs::path geom = global::resolvePath("src/shader/convolution/shader.geom");

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
    fs::path frag = global::resolvePath("src/shader/convolution/irradiance.frag");
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
    fs::path frag = global::resolvePath("src/shader/convolution/prefilter.frag");
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

namespace util::convolution::brdf {
    fs::path frag = global::resolvePath("src/shader/convolution/BRDF.frag");
    Shader shader(vert.c_str(), frag.c_str());
}

/**
 * BRDF积分的 Look-Up Table 贴图生成
 * @param resolution 生成的贴图分辨率
 * @return BRDF LUT 贴图
 */
GLuint util::generateBRDFLUT(unsigned int resolution) {
    unsigned int LUT, FBO;

    glGenTextures(1, &LUT);
    glGenFramebuffers(1, &FBO);
    glBindTexture(GL_TEXTURE_2D, LUT);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // use GL_RG16F to pre-allocate enough memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resolution, resolution, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LUT, 0);

    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    brdf::shader.use();
    global::drawQuadra();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return LUT;
}

namespace util::convolution::bilateral_filter {
    fs::path frag = global::resolvePath("src/shader/convolution/BilateralFilter.frag");
    fs::path cubemapFrag = global::resolvePath("src/shader/convolution/BilateralFilterCubemap.frag");

    Shader shader(vert.c_str(), frag.c_str());
    Shader cubemapShader(vert.c_str(), cubemapFrag.c_str(), geom.c_str());
}

/**
 * 双边滤波器
 * @param image  输入值贴图
 * @param width  生成的贴图宽度分辨率
 * @param height 生成的贴图高度分辨率
 * @return 通过滤波器后生成的贴图
 */
GLuint util::bilateralFilter(GLuint &image, unsigned int width, unsigned int height) {
    GLuint FBO, filterMap;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &filterMap);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_2D, filterMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, filterMap, 0);

    bilateral_filter::shader.use();
    bilateral_filter::shader.setInt("image", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image);

    global::drawQuadra();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return filterMap;
}

/**
 * 双边滤波器(Cubemap版)
 * @param cubemap    输入值贴图
 * @param resolution 生成的立方体贴图的每一面的分辨率
 * @return 通过滤波器后生成的立方体贴图
 */
GLuint util::bilateralFilterCubemap(GLuint &cubemap, unsigned int resolution) {
    GLuint FBO, filterMap;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &filterMap);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, filterMap);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, filterMap, 0);

    bilateral_filter::cubemapShader.use();
    bilateral_filter::cubemapShader.setInt("image", 0);
    bilateral_filter::cubemapShader.setMat4("projection", projection);

    for (unsigned int i = 0; i < 6; i++)
        bilateral_filter::cubemapShader.setMat4("views[" + std::to_string(i) + "]", views[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    global::drawCube();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return filterMap;
}

namespace util::convolution::gaussian_filter {
    fs::path cubemapFrag = global::resolvePath("src/shader/convolution/GaussianFilterCubemap.frag");
    Shader cubemapShader(vert.c_str(), cubemapFrag.c_str(), geom.c_str());
}

/**
 * 高斯滤波器(Cubemap版)
 * @param cubemap    输入值贴图
 * @param resolution 生成的立方体贴图的每一面的分辨率
 * @param radius     用于计算高斯滤波覆盖的区域半径，计算方式：0.5π / radius
 * @return 通过滤波器后生成的立方体贴图
 */
GLuint util::gaussianFilterCubemap(GLuint &cubemap, unsigned int resolution, unsigned int radius) {
    GLuint FBO, filterMap;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &filterMap);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, filterMap);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, filterMap, 0);

    gaussian_filter::cubemapShader.use();
    gaussian_filter::cubemapShader.setInt("image", 0);
    gaussian_filter::cubemapShader.setFloat("radius", (float)radius);
    gaussian_filter::cubemapShader.setMat4("projection", projection);

    for (unsigned int i = 0; i < 6; i++)
        gaussian_filter::cubemapShader.setMat4("views[" + std::to_string(i) + "]", views[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    global::drawCube();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return filterMap;
}

namespace util::convolution::inverse_tone_mapping {
    fs::path frag = global::resolvePath("src/shader/convolution/InverseToneMapping.frag");
    Shader shader(vert.c_str(), frag.c_str(), geom.c_str());
}

/**
 * 逆色调映射
 * @param LDR         输入LDR贴图
 * @param sigma       σ参数贴图
 * @param surrounding 局部光强贴图
 * @param maxValue    HDR贴图的最大值
 * @param resolution  生成的立方体贴图的每一面的分辨率
 * @return 生成的HDR贴图
 */
GLuint util::inverseToneMapping(GLuint &LDR, GLuint &sigma, GLuint &surrounding, float maxValue, unsigned int resolution) {
    GLuint FBO, HDR;

    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &HDR);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, HDR);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, HDR, 0);

    inverse_tone_mapping::shader.use();
    inverse_tone_mapping::shader.setInt("image", 0);
    inverse_tone_mapping::shader.setInt("sigmaMap", 1);
    inverse_tone_mapping::shader.setInt("surroundingMap", 2);
    inverse_tone_mapping::shader.setFloat("maxValue", maxValue);
    inverse_tone_mapping::shader.setMat4("projection", projection);

    for (unsigned int i = 0; i < 6; i++)
        inverse_tone_mapping::shader.setMat4("views[" + std::to_string(i) + "]", views[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, LDR);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sigma);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, surrounding);

    global::drawCube();

    glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &FBO);

    return HDR;
}
