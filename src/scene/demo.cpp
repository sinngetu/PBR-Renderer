#include <renderer/global.h>
#include <renderer/model.h>
#include <renderer/shadow.h>
#include <renderer/skybox.h>

#include "scene.h"

#include "../material/material.h"
#include "../util/util.h"

using namespace glm;

namespace scene::demo {
    fs::path lenaPath = global::resolvePath("assets/lena.png");
    fs::path skyboxPath = global::resolvePath("assets/skybox/1.png");
    fs::path modelPath = global::resolvePath("assets/PreviewSphere/Model.obj");

    fs::path BaseColor = global::resolvePath("assets/PreviewSphere/BaseColor.png");
    fs::path Metallic = global::resolvePath("assets/PreviewSphere/Metallic.png");
    fs::path Roughness = global::resolvePath("assets/PreviewSphere/Roughness.png");
    fs::path Normal = global::resolvePath("assets/PreviewSphere/Normal.png");
    fs::path Height = global::resolvePath("assets/PreviewSphere/Height.png");
    fs::path AO = global::resolvePath("assets/PreviewSphere/AO.png");

    Model model(modelPath.c_str());
    Shadow shadow;
    Skybox skybox(skyboxPath.c_str());
    // material::Default mtl;
    material::PBR mtl;
    util::Debug debug;

    vec3 lightDirection(-2.0f, 4.0f, -1.0f);
    vec3 lightColor = vec3(10.0) * vec3(0.8549f, 0.5843f, 0.5882f);

    std::vector<Model> models;

    unsigned int FBO, colorBuffer, depth;
    void init() {
        // GLuint envMap = global::loadCubemap(skyboxPath.c_str());
        // GLuint gaussianFilterMap = envMap;
        // for(unsigned int i = 0; i < 10; i++)
        //     gaussianFilterMap = util::gaussianFilterCubemap(gaussianFilterMap);
        // GLuint bilateralFilterMap = util::bilateralFilterCubemap(envMap);
        // GLuint HDRMap = util::inverseToneMapping(envMap, gaussianFilterMap, bilateralFilterMap);

        // skybox.setCubemap(HDRMap);

        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, global::SCREEN_WIDTH, global::SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

using namespace scene::demo;

scene::Demo::Demo() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    init();
    mat4 M = mat4(1.0f);
    M = translate(M, vec3(0.0f, -1.0f, 0.0f));

    model.setM(M);
    models.push_back(model);

    skybox.init();

    mtl.setup(skybox.getCubemap(), BaseColor.c_str(), Metallic.c_str(), Roughness.c_str(), Normal.c_str(), Height.c_str(), AO.c_str());
    mtl.setHeightScale(1.0);
    mtl.setLight(lightDirection, lightColor);
    shadow.setPorps(lightDirection, vec3(0.0f, 1.0f, 0.0f), 2.5f);
}

void scene::Demo::loop() {
    glClearColor(0.231f, 0.345f, 0.459f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene();
    postProcessing();
}

void scene::Demo::renderScene() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    mat4 view = global::camera.GetViewMatrix();
    mat4 projection = perspective(radians(global::camera.Zoom), (float)global::SCREEN_WIDTH / (float)global::SCREEN_HEIGHT, 0.1f, 100.0f);

    shadow.shadowMapping(models);
    shadow.setMap();

    mtl.setVP(view, projection);
    mtl.setViewPositon(global::camera.Position);
    // mtl.setShadow(shadow.getWorldToLight());
    mtl.setShadow(shadow.getMap(), shadow.getWorldToLight());
    mtl.setTextures();

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.2f, 0.1f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene::demo::model.Draw(mtl);
    skybox.Draw(view, projection);

    // transparent queue
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render transparent objects
}

void scene::Demo::postProcessing() {
    GLuint buffer;
    buffer = util::Bloom(colorBuffer);
    buffer = util::Correction(buffer, true);
}
