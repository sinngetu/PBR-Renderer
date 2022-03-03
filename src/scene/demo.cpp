#include <renderer/global.h>
#include <renderer/model.h>
#include <renderer/shadow.h>
#include <renderer/skybox.h>

#include "scene.h"

#include "../util/util.h"
#include "../util/PBRModel.h"

using namespace glm;

namespace scene::demo {
    fs::path lenaPath = global::resolvePath("assets/lena.png");
    fs::path skyboxPath = global::resolvePath("assets/skybox/1.png");
    fs::path thepath = global::resolvePath("assets/PreviewSphere");

    PBRModel themodel(thepath.c_str());
    Shadow shadow;
    Skybox skybox(skyboxPath.c_str());

    DirectionalLight light = {
        .direction = vec3(-2.0f, 4.0f, -1.0f),
        .color = vec3(10.0) * vec3(0.8549f, 0.5843f, 0.5882f)
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
}

namespace scene::demo::model {
    std::vector<const char*> geometryPath = {
        "assets/PBR/PreviewSphere"
    };

    std::vector<const char*> transparentPath = {};

    std::vector<Model> projection;
    std::vector<PBRModel> geometry, transparent;

    void init() {
        for(unsigned int i = 0; i < geometryPath.size(); i++) {
            fs::path modelPath = global::resolvePath(geometryPath[i]);

            geometry.push_back(PBRModel(modelPath.c_str()));
            geometry[i].setup(skybox.getCubemap());
            projection.push_back(geometry[i]);
        }

        for(unsigned int i = 0; i < transparentPath.size(); i++) {
            fs::path modelPath = global::resolvePath(transparentPath[i]);

            transparent.push_back(PBRModel(modelPath.c_str()));
            transparent[i].setup(skybox.getCubemap());
        }
    }
}

using namespace scene::demo;

scene::Demo::Demo() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    init();
    skybox.init();
    PBRModel::init(light);
    model::init();

    shadow.setPorps(light.direction, vec3(0.0f, 1.0f, 0.0f), 2.5f);
}

void scene::Demo::loop() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
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

    shadow.shadowMapping(model::projection);
    shadow.setMap();

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.2f, 0.1f, 0.0f, 1.0f);
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
    GLuint buffer;
    buffer = util::Bloom(colorBuffer);
    buffer = util::Correction(buffer, true);
}
