#ifndef MATERIALS_H
#define MATERIALS_H

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

class PBR: public Material {
private:
    float heightScale = 0.0;
    unsigned int irradianceMap, prefilterMap, brdfLUT = 0,
                 baseColorMap, metallicMap, roughnessMap, normalMap, heightMap, aoMap;

public:
    PBR();
    void setup();
    void setHeightScale(float scale);
    void setLight(vec3 direction, vec3 color);
    void setShadow(GLuint shadowMap, mat4 WorldToLight);
};
}
#endif
