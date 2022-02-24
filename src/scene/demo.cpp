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
    fs::path skyboxPath = global::resolvePath("assets/skybox/3.png");

    Shadow shadow;
    Skybox skybox;
    material::Default mtl;
    util::Debug debug;

    vec3 lightDirection(-2.0f, 4.0f, -1.0f);
    vec3 lightColor(10.0f);

    std::vector<Model> models;
    unsigned int lena, filterMap;
}

using namespace scene::demo;

scene::Demo::Demo() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    lena = global::loadTexture(lenaPath.c_str());
    filterMap = util::bilateralFilter(lena);
    // skybox.setCubemap(filterMap);

    mat4 M = mat4(1.0f);
    M = translate(M, vec3(0.0f, -1.0f, 0.0f));

    // model.setM(M);
    // models.push_back(model);

    mtl.setLight(lightDirection, lightColor);
    shadow.setPorps(lightDirection, vec3(0.0f, 1.0f, 0.0f), 2.5f);
}

void scene::Demo::loop() {
    glClearColor(0.231f, 0.345f, 0.459f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    mat4 view = global::camera.GetViewMatrix();
    mat4 projection = perspective(radians(global::camera.Zoom), (float)global::SCREEN_WIDTH / (float)global::SCREEN_HEIGHT, 0.1f, 100.0f);

    // shadow.shadowMapping(models);
    // shadow.setMap();

    // mtl.setVP(view, projection);
    // mtl.setViewPositon(global::camera.Position);
    // mtl.setShadow(shadow.getWorldToLight());

    // scene::demo::model.Draw(mtl);
    // skybox.Draw(view, projection);

    debug.render(filterMap);
    // debug.render(shadow.getMap());
}
