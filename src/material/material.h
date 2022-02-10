#include <renderer/shader.h>
#include <renderer/material.h>
#include <renderer/model.h>

using namespace glm;

namespace material {
class Shadow {
private:
    GLuint depthMap, depthMapFBO;
    bool inited = false;
    Shader shadowShader;
    unsigned int textureIndex;
    vec3 direction;
    vec3 offset;
    GLfloat halfWidth;
    GLfloat near;
    GLfloat far;

    void init();

protected:
    mat4 WorldToLight;
    void setShadowTextureIndex(unsigned int i);

public:
    Shadow();
    void setShadowPorps(vec3 direction, vec3 offset = vec3(0.0f), GLfloat halfWidth = 10.0f, GLfloat near = 0.1f, GLfloat far = 100.0f);
    void shadowMapping(std::vector<Model> models);
    void setShadowMap();
    GLuint getShadowMap();
    mat4 getWorldToLight();
};

class Default: public Material, public Shadow {
public:
    Default();
    void init();
    void setLight(vec3 direction, vec3 color);
    void setShadow();
};

class Debug: public Material {
public:
    Debug();
};
}
