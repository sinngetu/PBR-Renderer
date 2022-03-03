#ifndef PBR_MODEL
#define PBR_MODEL

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <renderer/model.h>
#include <renderer/shadow.h>
#include <renderer/global.h>

#include "util.h"
#include "../material/material.h"

namespace PBR {
    material::PBR mtl;
    GLuint brdfLUT;
}

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

class PBRModel : public Model {
private:
    const char *path;
    GLuint irradianceMap, prefilterMap;
    GLuint baseColorMap, metallicMap, roughnessMap, normalMap, heightMap, aoMap;

    void setTextures() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, baseColorMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metallicMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, heightMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, aoMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, PBR::brdfLUT);
    }

public:
    PBRModel(const char *path) : Model((std::string(path) + std::string("/Model.obj")).c_str()) {
        this->path = path;
    }

    void Render() {
        PBR::mtl.use();
        PBR::mtl.setM(M);
        setTextures();

        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw();
    }

    void setup(GLuint envMap) {
        baseColorMap = global::loadTexture((std::string(path) + std::string("/BaseColor.png")).c_str());
        metallicMap = global::loadTexture((std::string(path) + std::string("/Metallic.png")).c_str());
        roughnessMap = global::loadTexture((std::string(path) + std::string("/Roughness.png")).c_str());
        normalMap = global::loadTexture((std::string(path) + std::string("/Normal.png")).c_str());
        heightMap = global::loadTexture((std::string(path) + std::string("/Height.png")).c_str());
        aoMap = global::loadTexture((std::string(path) + std::string("/AO.png")).c_str());

        GLuint gaussianFilterMap = envMap;
        for(unsigned int i = 0; i < 10; i++)
            gaussianFilterMap = util::gaussianFilterCubemap(gaussianFilterMap);

        GLuint bilateralFilterMap = util::bilateralFilterCubemap(envMap);
        GLuint HDRMap = util::inverseToneMapping(envMap, gaussianFilterMap, bilateralFilterMap);

        irradianceMap = util::generateIrradianceMap(HDRMap);
        prefilterMap = util::generatePrefilterMap(HDRMap);
    }

    static void init(DirectionalLight light, float heightScale = 1.0f) {
        PBR::mtl.setup();
        PBR::mtl.setLight(light.direction, light.color);
        PBR::mtl.setHeightScale(heightScale);
        PBR::brdfLUT = util::generateBRDFLUT();
    }

    static void setVP(glm::mat4 view, glm::mat4 projection) {
        PBR::mtl.setVP(view, projection);
    }

    static void setViewPositon(glm::vec3 position = global::camera.Position) {
        PBR::mtl.setViewPositon(position);
    }

    static void setShadow(Shadow &shadow) {
        PBR::mtl.setShadow(shadow.getMap(), shadow.getWorldToLight());
    }
};
#endif
