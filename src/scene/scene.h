#ifndef SCENE_H
#define SCENE_H

class Scene {
public:
    virtual void loop() {};
};

class Demo : public Scene {
public:
    Demo();
    void loop();
};
#endif
