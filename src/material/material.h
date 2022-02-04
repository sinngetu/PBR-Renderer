#include <renderer/material.h>

using namespace glm;

namespace material {
class Default: public Material {
public:
    Default();
    void setMVP(mat4 M, mat4 V, mat4 P);
    void setCameraPositon(vec3 pos);
    void setLight(vec3 direction, vec3 color);
};
}
