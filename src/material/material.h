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
    void setup(
        GLuint envMap,
        const char* baseColor,
        const char* metallic,
        const char* roughness,
        const char* normal,
        const char* height,
        const char* ao
    );

    void setTextures();
    void setHeightScale(float scale);
    void setLight(vec3 direction, vec3 color);
    void setShadow(GLuint shadowMap, mat4 WorldToLight);
};
}
