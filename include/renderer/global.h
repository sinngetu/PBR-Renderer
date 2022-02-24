#ifndef GLOBAL_H
#define GLOBAL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>

#include <renderer/camera.h>

namespace fs = std::filesystem;
namespace global {
extern const char* root;

// window
extern GLFWwindow* window;

extern unsigned int SCREEN_WIDTH;
extern unsigned int SCREEN_HEIGHT;
extern unsigned int SHADOW_WIDTH;
extern unsigned int SHADOW_HEIGHT;

// camera
extern Camera camera;
extern float lastX;
extern float lastY;

// timing
extern float deltaTime;
extern float lastFrame;

extern void inputHandle();
extern void onMouseMove(GLFWwindow* window, double xpos, double ypos);
extern void onScroll(GLFWwindow* window, double xoffset, double yoffset);
extern void framebufferSizeCallback(GLFWwindow *window, int width, int height);
extern std::filesystem::path resolvePath(const char *path);
extern bool init();
extern GLuint loadTexture(char const* path, bool flipY = true);
extern GLuint loadCubemap(const char *cubemap);
extern GLuint loadCubemap(std::vector<std::string> faces);
extern void drawCube();
extern void drawQuadra();
};

#endif
