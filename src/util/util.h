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

GLuint generateIrradianceMap(GLuint &cubemap, unsigned int resolutionRatio = 32);
GLuint generatePrefilterMap(GLuint &cubemap, unsigned int resolutionRation = 128);
GLuint generateBRDFLUT(unsigned int resolution = 512);
GLuint bilateralFilter(GLuint &image, unsigned int width = global::SCREEN_WIDTH, unsigned int height = global::SCREEN_HEIGHT);
GLuint bilateralFilterCubemap(GLuint &cubemap, unsigned int resolution = 1024);
}
#endif
