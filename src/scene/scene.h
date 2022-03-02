#ifndef SCENE_H
#define SCENE_H

class Scene {
public:
    virtual void loop() {};
};

namespace scene {
class Demo : public Scene {
private:
    void renderScene();
    void postProcessing();

public:
    Demo();
    void loop();
};

class Debug : public Scene {
public:
    Debug();
    void loop();
};
}
#endif
