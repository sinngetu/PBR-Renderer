#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>
#include <renderer/global.h>

namespace util {
class Debug {
private:
    bool singleColor;
    void init();

public:
    Debug() { singleColor = false; };
    Debug(bool singleColor) { this->singleColor = singleColor; };
    void render(GLuint texture);
};

namespace convolution {
    GLuint generateIrradianceMap(GLuint &cubemap, unsigned int resolutionRatio = 32, GLuint FBO = 0);
    GLuint generatePrefilterMap(GLuint &cubemap, unsigned int resolutionRation = 128);
    GLuint generateBRDFLUT(unsigned int resolution = 512, GLuint FBO = 0);
    GLuint bilateralFilter(GLuint &image, unsigned int width = global::SCREEN_WIDTH, unsigned int height = global::SCREEN_HEIGHT);
    GLuint bilateralFilterCubemap(GLuint &cubemap, unsigned int resolution = 1024);
    GLuint gaussianFilter(GLuint &image, unsigned int width = global::SCREEN_WIDTH, unsigned int height = global::SCREEN_HEIGHT);
    GLuint gaussianFilterCubemap(GLuint &cubemap, unsigned int resolution = 512, unsigned int radius = 256);
    GLuint inverseToneMapping(GLuint &LDR, GLuint &sigma, GLuint &surrounding, float maxValue = 10.0, unsigned int resolution = 1024);
}

namespace postProcessing {
    GLuint Bloom(GLuint &image, float threshold = 1.0);
    GLuint Correction(GLuint &image, bool toScreen = false, bool toneMapping = false, float exposure = 1.0, GLuint FBO = 0);
}

namespace write {
    void image(GLuint &FBO, const char *filename, GLsizei width, GLsizei height, GLsizei channels = 3, GLenum format = GL_RGB, const char *outputDir = "export/");
    void cubemap(GLuint &cubemap, GLuint &FBO, const char *filename, GLsizei resolution, GLsizei channels = 3, GLenum format = GL_RGB, const char *outputDir = "export/");
}
}
#endif
