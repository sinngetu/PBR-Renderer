#include <glm/glm.hpp>

#include <renderer/global.h>

class Skybox {
private:
    GLuint VBO, VAO, shader, cubemap = 0;
    const char *path;

    void init() {
        global::loadCubemap(path, &cubemap);

        float vertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

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
            "out vec4 ST_Target;\n"

            "uniform samplerCube skybox;\n"

            "void main() {\n"
            "    ST_Target = texture(skybox, uv);\n"
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

public:
    Skybox(const char *skybox) {
        path = skybox;
    }

    ~Skybox() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    GLuint getCubemap() {
        return cubemap;
    }

    void Draw(glm::mat4 view, glm::mat4 projection) {
        if (cubemap == 0) init();

        // remove translation data
        view = glm::mat4(glm::mat3(view));

        glDepthFunc(GL_LEQUAL);

        glUseProgram(shader);
        glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }
};
