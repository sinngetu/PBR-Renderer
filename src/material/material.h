#include <renderer/shader.h>
#include <renderer/material.h>
#include <renderer/model.h>

using namespace glm;

namespace material {
class Default: public Material {
public:
    Default();
    void setLight(vec3 direction, vec3 color);
    void setShadow(mat4 WorldToLight);
};

class Debug: public Material {
public:
    Debug();
};
}
