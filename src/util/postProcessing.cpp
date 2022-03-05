#include <glad/glad.h>

#include <renderer/shader.h>
#include <renderer/global.h>

#include "util.h"

namespace util::postProcessing {
    fs::path vert = global::resolvePath("src/shader/postProcessing/shader.vert");
}

using namespace util::postProcessing;

namespace util::postProcessing::bloom {
    fs::path brightFrag = global::resolvePath("src/shader/postProcessing/bloom/bright.frag");
    fs::path bloomFrag = global::resolvePath("src/shader/postProcessing/bloom/bloom.frag");
    fs::path fusionFrag = global::resolvePath("src/shader/postProcessing/bloom/fusion.frag");

    Shader brightShader(vert.c_str(), brightFrag.c_str());
    Shader bloomShader(vert.c_str(), bloomFrag.c_str());
    Shader fusionShader(vert.c_str(), fusionFrag.c_str());

    unsigned int colorBuffers[2], FBO[2] = {0, 0};
}

/**
 * 泛光
 * @param image     处理图片
 * @param threshold 泛光部分的亮度阈值
 * @return 处理后的图片
 */
GLuint util::postProcessing::Bloom(GLuint &image, float threshold) {
    if (bloom::FBO[0] == 0) {
        glGenFramebuffers(2, bloom::FBO);
        glGenTextures(2, bloom::colorBuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, bloom::FBO[i]);
            glBindTexture(GL_TEXTURE_2D, bloom::colorBuffers[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, global::SCREEN_WIDTH, global::SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom::colorBuffers[i], 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "util::postProcessing::Bloom: Framebuffer not complete!" << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, bloom::FBO[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image);
    bloom::brightShader.use();
    bloom::brightShader.setInt("image", 0);
    bloom::brightShader.setFloat("threshold", threshold);
    global::drawQuadra();

    bloom::bloomShader.use();
    bloom::bloomShader.setInt("image", 0);
    for (unsigned int i = 0; i < 10; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloom::FBO[i % 2]);
        glBindTexture(GL_TEXTURE_2D, bloom::colorBuffers[(i + 1) % 2]);
        global::drawQuadra();
    }

    bloom::fusionShader.use();
    bloom::fusionShader.setInt("image", 0);
    bloom::fusionShader.setInt("bloom", 1);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom::FBO[0]);
    glBindTexture(GL_TEXTURE_2D, image);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom::colorBuffers[1]);
    global::drawQuadra();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return bloom::colorBuffers[0];
}

namespace util::postProcessing::correction {
    fs::path frag = global::resolvePath("src/shader/postProcessing/Correction.frag");
    Shader shader(vert.c_str(), frag.c_str());

    unsigned int colorBuffer, FBO = 0;
}

/**
 * Gamma校正
 * @param image       处理图片
 * @param toScreen    是否直接输出到屏幕，即 BindFramebuffer 为 0
 * @param toneMapping 是否需要色调映射
 * @param exposure    色调映射的曝光度
 * @param FBO         渲染使用的 framebuffer，优先级低于 toScreen
 * @return 处理后的图片
 */
GLuint util::postProcessing::Correction(GLuint &image, bool toScreen, bool toneMapping, float exposure, GLuint FBO) {
    if (correction::FBO == 0) {
        glGenFramebuffers(1, &correction::FBO);
        glGenTextures(1, &correction::colorBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, correction::FBO);
        glBindTexture(GL_TEXTURE_2D, correction::colorBuffer);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, global::SCREEN_WIDTH, global::SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, correction::colorBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "util::postProcessing::Correction: Framebuffer not complete!" << std::endl;
    }

    FBO = FBO == 0 ? correction::FBO : FBO;

    if (toScreen) glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, toScreen ? 0 : FBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image);
    correction::shader.use();
    correction::shader.setInt("image", 0);
    correction::shader.setBool("toneMapping", toneMapping);
    correction::shader.setFloat("exposure", exposure);
    global::drawQuadra();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return toScreen ? 0 : correction::colorBuffer;
}
