#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>

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
}
#endif
