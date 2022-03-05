#ifndef SCENE_H
#define SCENE_H

#include <renderer/global.h>

using namespace glm;

class Scene {
public:
    virtual void loop() {};
};

namespace scene {
class Demo : public Scene {
public:
    Demo();
    void loop();
    void renderScene(
        GLuint FBO,
        mat4 view = global::camera.GetViewMatrix(),
        mat4 projection = perspective(radians(global::camera.Zoom), (float)global::SCREEN_WIDTH / (float)global::SCREEN_HEIGHT, 0.1f, 100.0f)
    );
    void postProcessing();
};

class Debug : public Scene {
public:
    Debug();
    void loop();
};
}
#endif
