#include <renderer/global.h>
#include <renderer/model.h>
#include <renderer/shadow.h>

#include "scene.h"

#include "../material/material.h"
#include "../util/util.h"

using namespace glm;

namespace scene::demo {
    fs::path modelPath = global::resolvePath("assets/decoretive.obj");

    Model model(modelPath.c_str());
    Shadow shadow;
    material::Default mtl;
    util::Debug debug;

    vec3 lightDirection(-2.0f, 4.0f, -1.0f);
    vec3 lightColor(0.95f);

    std::vector<Model> models;
}

using namespace scene::demo;

scene::Demo::Demo() {
    mat4 M = mat4(1.0f);
    M = translate(M, vec3(0.0f, -1.0f, 0.0f));

    model.setM(M);
    models.push_back(model);

    mtl.setLight(lightDirection, lightColor);
    shadow.setPorps(lightDirection, vec3(0.0f, 1.0f, 0.0f), 2.5f);
}

void scene::Demo::loop() {
    glClearColor(0.231f, 0.345f, 0.459f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    mat4 view = global::camera.GetViewMatrix();
    mat4 projection = perspective(radians(global::camera.Zoom), (float)global::SCREEN_WIDTH / (float)global::SCREEN_HEIGHT, 0.1f, 100.0f);

    shadow.shadowMapping(models);
    shadow.setMap();

    mtl.setVP(view, projection);
    mtl.setViewPositon(global::camera.Position);
    mtl.setShadow(shadow.getWorldToLight());

    scene::demo::model.Draw(mtl);

    // debug.render(shadow.getMap());
}
