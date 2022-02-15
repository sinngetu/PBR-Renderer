#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <renderer/global.h>
#include <renderer/model.h>

class Shadow {
private:
    GLuint depthMap, depthMapFBO;
    GLuint shader = 0;
    unsigned int textureIndex;
    glm::mat4 WorldToLight;
    glm::vec3 direction;
    glm::vec3 offset;
    GLfloat halfWidth;
    GLfloat near;
    GLfloat far;

    void init() {
        const char *vertCode =
            "#version 410 core\n"
            "layout (location = 0) in vec3 positionOS;\n"

            "uniform mat4 WorldToLight;\n"
            "uniform mat4 model;\n"

            "void main() {\n"
            "    vec3 positionWS = (model * vec4(positionOS, 1.0)).xyz;\n"
            "    gl_Position = WorldToLight * vec4(positionWS, 1.0);\n"
            "}";

        const char *fragCode =
            "#version 410 core\n"
            "void main() {}";

        // init shader
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertCode, NULL);
        glCompileShader(vertex);

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragCode, NULL);
        glCompileShader(fragment);

        // create shader program
        shader = glCreateProgram();
        glAttachShader(shader, vertex);
        glAttachShader(shader, fragment);

        glLinkProgram(shader);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        // init depth map
        glGenFramebuffers(1, &depthMapFBO);
        glGenTextures(1, &depthMap);

        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, global::SHADOW_WIDTH, global::SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // border sampling handle
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // init FBO
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

public:
    ~Shadow() { glDeleteProgram(shader); }

    void setPorps(glm::vec3 direction, glm::vec3 offset = glm::vec3(0.0f), GLfloat halfWidth = 10.0f, GLfloat near = 0.1f, GLfloat far = 100.0f) {
        if(shader == 0) init();

        this->direction = direction;
        this->offset = offset;
        this->halfWidth = halfWidth;
        this->near = near;
        this->far = far;
    }

    void shadowMapping(std::vector<Model> models) {
        glm::mat4 projection = glm::ortho(-halfWidth, halfWidth, -halfWidth, halfWidth, near, far);
        glm::mat4 view = glm::lookAt(direction + offset, offset, glm::vec3(0.0, 1.0, 0.0));
        WorldToLight = projection * view;

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "WorldToLight"), 1, GL_FALSE, glm::value_ptr(WorldToLight));

        glViewport(0, 0, global::SHADOW_WIDTH, global::SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        for(unsigned int i = 0; i < models.size(); i++) {
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(models[i].getM()));
            models[i].Draw();
        }

        // reset
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, global::SCREEN_WIDTH, global::SCREEN_HEIGHT);
    }

    void setMap() {
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    GLuint getMap() {
        return depthMap;
    }

    glm::mat4 getWorldToLight() {
        return WorldToLight;
    }

    void setTextureIndex(unsigned int i) {
        textureIndex = i;
    }
};
