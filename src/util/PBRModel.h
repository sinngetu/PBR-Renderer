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
    GLuint brdfLUT, irradianceMap, prefilterMap;
}

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

class PBRModel : public Model {
private:
    const char *path;
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
        glBindTexture(GL_TEXTURE_CUBE_MAP, PBR::irradianceMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, PBR::prefilterMap);
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

    void setup() {
        baseColorMap = global::loadTexture((std::string(path) + std::string("/BaseColor.png")).c_str());
        metallicMap = global::loadTexture((std::string(path) + std::string("/Metallic.png")).c_str());
        roughnessMap = global::loadTexture((std::string(path) + std::string("/Roughness.png")).c_str());
        normalMap = global::loadTexture((std::string(path) + std::string("/Normal.png")).c_str());
        heightMap = global::loadTexture((std::string(path) + std::string("/Height.png")).c_str());
        aoMap = global::loadTexture((std::string(path) + std::string("/AO.png")).c_str());
    }

    static void init(GLuint envMap, DirectionalLight light, float heightScale = 1.0f) {
        PBR::mtl.setup();
        PBR::mtl.setLight(light.direction, light.color);
        PBR::mtl.setHeightScale(heightScale);

        auto brdfLUT_path = global::resolvePath("export/BRDF_LUT.png");
        PBR::brdfLUT = global::loadTexture(brdfLUT_path.c_str());

        std::vector<std::string> faces;
        fs::path irradiancePaths[6] = {
            global::resolvePath("export/irradiance_1.png"),
            global::resolvePath("export/irradiance_2.png"),
            global::resolvePath("export/irradiance_3.png"),
            global::resolvePath("export/irradiance_4.png"),
            global::resolvePath("export/irradiance_5.png"),
            global::resolvePath("export/irradiance_6.png"),
        };

        for(unsigned int i = 0; i < 6; i++)
            faces.push_back(irradiancePaths[i].string());

        PBR::irradianceMap = global::loadCubemap(faces);
        PBR::prefilterMap = util::convolution::generatePrefilterMap(envMap);
    }

    static void setVP(glm::mat4 view, glm::mat4 projection) {
        PBR::mtl.setVP(view, projection);
    }

    static void setViewPositon(glm::vec3 position = global::camera.Position) {
        PBR::mtl.setViewPositon(position);
    }

    static void setShadow(Shadow &shadow) {
        PBR::mtl.setShadow(shadow.getMap(), shadow.getWorldToLight(), shadow.getBias());
    }
};
#endif
