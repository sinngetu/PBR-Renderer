#include <renderer/global.h>
#include <renderer/model.h>
#include <renderer/shadow.h>
#include <renderer/skybox.h>

#include "scene.h"

#include "../material/material.h"
#include "../util/util.h"
#include "../util/PBRModel.h"

using namespace glm;

namespace scene::demo {
    fs::path lenaPath = global::resolvePath("assets/lena.png");
    fs::path skyboxPath = global::resolvePath("assets/skybox/3.png");

    Shadow shadow;
    Skybox skybox(skyboxPath.c_str());

    DirectionalLight light = {
        .direction = vec3(1.0f, 0.2f, 3.0f),
        .color = vec3(15.0) * vec3(1.0f, 1.0f, 1.0f)
    };

    unsigned int FBO, colorBuffer, depth;
    void init() {
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

    GLuint texture;
    util::Debug debug;
}

namespace scene::demo::model {
    std::vector<const char*> geometryPath = {
        "assets/PBR/bay_window",
        "assets/PBR/books",
        "assets/PBR/bookshelf_1",
        "assets/PBR/bookshelf_2",
        "assets/PBR/chandelier",
        "assets/PBR/clock",
        "assets/PBR/pot_plant",
        "assets/PBR/room",
        "assets/PBR/sofa",
        "assets/PBR/table",
        "assets/PBR/window_frame"
    };

    std::vector<const char*> transparentPath = {
        "assets/PBR/window"
    };

    std::vector<Model> projection;  // 生成阴影投影的模型
    std::vector<PBRModel> geometry, transparent; // 不透明物体，和透明物体

    void init() {
        for(unsigned int i = 0; i < geometryPath.size(); i++) {
            auto modelPath = global::resolvePath(geometryPath[i]);
            auto model = PBRModel(modelPath.c_str());

            model.setup();
            geometry.push_back(model);
            projection.push_back(model);
        }

        for(unsigned int i = 0; i < transparentPath.size(); i++) {
            auto modelPath = global::resolvePath(transparentPath[i]);
            auto model = PBRModel(modelPath.c_str());

            model.setup();
            transparent.push_back(model);
        }
    }
}

using namespace scene::demo;

scene::Demo::Demo() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    init();
    skybox.init();
    model::init();
    PBRModel::init(skybox.getCubemap(), light);

    mat4 M = mat4(1.0f);
    M = rotate(M, radians(-130.0f), vec3(0.0f, 1.0f, 0.0f));

    skybox.setM(M);

    shadow.setPorps(vec3(8.5f, 14.3f, 20.0f), vec3(-1.5f, 10.0f, 0.0f), 11.0f, 0.001f);
}

void scene::Demo::loop() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene(0);
    // renderScene(demo::FBO);
    // postProcessing();
}

void scene::Demo::renderScene(GLuint FBO, mat4 view, mat4 projection) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    shadow.shadowMapping(model::projection);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PBRModel::setVP(view, projection);
    PBRModel::setViewPositon();
    PBRModel::setShadow(shadow);

    for(unsigned int i = 0; i < model::geometry.size(); i++)
        model::geometry[i].Render();

    skybox.Draw(view, projection);

    // transparent queue
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: sort transparent objects
    for(unsigned int i = 0; i < model::transparent.size(); i++)
        model::transparent[i].Render();
}

void scene::Demo::postProcessing() {
    GLuint buffer = colorBuffer;
    // buffer = util::postProcessing::Bloom(buffer);
    util::postProcessing::Correction(buffer, true);
}
