#include <glm/glm.hpp>

#include <renderer/global.h>

class Skybox {
private:
    GLuint shader = 0, cubemap = 0;
    const char *path;

public:
    Skybox() {}
    Skybox(GLuint cubemap) { setCubemap(cubemap); }
    Skybox(const char *skybox) { path = skybox; }

    void setCubemap(GLuint cubemap) {
        this->cubemap = cubemap;
    }

    GLuint getCubemap() {
        return cubemap;
    }

    void init() {
        if (cubemap == 0)
            cubemap = global::loadCubemap(path);

        const char *vertCode =
            "#version 410 core\n"
            "layout (location = 0) in vec3 positionOS;\n"

            "out vec3 uv;\n"

            "uniform mat4 view;\n"
            "uniform mat4 projection;\n"

            "void main() {\n"
            "    vec4 positionCS = projection * view * vec4(positionOS, 1.0);\n"

            "    uv = positionOS;\n"
            "    gl_Position = positionCS.xyww;  // make sure the depth value is 1.0\n"
            "}";

        const char *fragCode =
            "#version 410 core\n"

            "in vec3 uv;\n"
            "out vec4 SV_Target;\n"

            "uniform samplerCube skybox;\n"

            "void main() {\n"
            "    SV_Target = texture(skybox, uv);\n"
            "}";

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
    }

    void Draw(glm::mat4 view, glm::mat4 projection) {
        if (shader == 0) init();

        // remove translation data
        view = glm::mat4(glm::mat3(view));

        glDepthFunc(GL_LEQUAL);

        glUseProgram(shader);
        glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        global::drawCube();

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }
};
