#include <glad/glad.h>
#include <iostream>

#include <renderer/model.h>
#include <renderer/global.h>

#include "scene.h"
#include "../material/material.h"

using namespace glm;

namespace demo {
    fs::path modelPath = global::resolvePath("assets/decoretive.obj");

    Model model(modelPath.c_str());
    material::Default mtl;

    vec3 lightDirection(-3.f, -3.0f, 1.0f);
    vec3 lightColor(0.95f);
}

using namespace demo;

Demo::Demo() {
    glEnable(GL_DEPTH_TEST);

    mtl.use();
    mtl.setLight(lightDirection, lightColor);
}

void Demo::loop() {
    glClearColor(0.231f, 0.345f, 0.459f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 model = mat4(1.0f);
    mat4 view = global::camera.GetViewMatrix();
    mat4 projection = perspective(radians(global::camera.Zoom), (float)global::SCREEN_WIDTH / (float)global::SCREEN_HEIGHT, 0.1f, 100.0f);

    model = translate(model, vec3(0.f, 0.f, 0.f));
    model = scale(model, vec3(0.01f));

    mtl.setMVP(model, view, projection);
    mtl.setCameraPositon(global::camera.Position);

    demo::model.Draw();
}
