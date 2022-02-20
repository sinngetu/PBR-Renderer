#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>

namespace util {
class Debug {
private: void init();
public: void render(GLuint texture);
};

GLuint generateIrradianceMap(GLuint &cubemap, unsigned int resolutionRatio = 32);
}
#endif
