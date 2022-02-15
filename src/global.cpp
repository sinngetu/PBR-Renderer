#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <renderer/camera.h>

using namespace glm;

namespace global {
// window
GLFWwindow* window;

unsigned int SCREEN_WIDTH = 800;
unsigned int SCREEN_HEIGHT = 600;
unsigned int SHADOW_WIDTH = 1024;
unsigned int SHADOW_HEIGHT = 1024;

// camera
Camera camera(vec3(1.0f, 0.5f, 4.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true, isInit = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput() {
    if(glfwGetKey(global::window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(global::window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(TOP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(BOTTOM, deltaTime);
}

void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void onScroll(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::filesystem::path resolvePath(const char *path) {
    std::filesystem::path result = std::filesystem::current_path();
    std::filesystem::path relativePath(path);

    result /= relativePath;

    return result;
}

bool init() {
    // init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // window
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "draw", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onScroll);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    return true;
}

void loadTexture(char const* path, unsigned int* id) {
    glGenTextures(1, id);

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);

    if (data) {
        GLenum format;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, *id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);
}

void loadCubemap(const char *cubemap, unsigned int* id) {
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *id);

    int width, height, channels;
    unsigned char *data = stbi_load(cubemap, &width, &height, &channels, 0);

    unsigned int coords[6][2] = {
        // {x, y}
        {2, 1},
        {0, 1},
        {1, 0},
        {1, 2},
        {1, 1},
        {3, 1}
    };

    int faceWidth = width / 4;
    int faceHeight = height / 3;
    auto faceData = (unsigned char *)malloc(faceWidth * faceHeight * channels);

    if(data) {
        GLenum format;
        if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        for(unsigned int i = 0; i < 6; i++) {
            int offsetX = coords[i][0] * faceWidth;
            int offsetY = coords[i][1] * faceHeight;

            for(unsigned int y = 0; y < faceHeight; y++) {
                for(unsigned int x = 0; x < faceWidth; x++) {
                    for(unsigned int chan = 0; chan < channels; chan++) {
                        int faceIndex = (y * faceWidth + x) * channels + chan;
                        int index = ((y + offsetY) * width + (x + offsetX)) * channels + chan;
                        faceData[faceIndex] = data[index];
                    }
                }
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faceData);
        }
    } else {
        std::cout << "Cubemap texture failed to load at path: " << cubemap << std::endl;
    }

    stbi_image_free(data);
    free(faceData);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void loadCubemap(std::vector<std::string> faces, unsigned int* id) {
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *id);

    int width, height, channels;

    for(unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

        if(data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
};
