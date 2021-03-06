#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <renderer/global.h>

#include "scene/scene.h"

using namespace global;

void loop(Scene *scene);

int main() {
    if (!init()) return -1;

    loop(new scene::Demo());

    glfwTerminate();
    return 0;
}

void loop(Scene *scene) {
    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputHandle();

        scene->loop();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
