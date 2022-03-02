#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
private:
    bool inited = false;
    unsigned int vertex, fragment, geometry;
    const GLchar *vertexPath, *fragmentPath, *geometryPath;

    void checkError(unsigned int *shader, const char *type) {
        GLint status;
        char infoLog[512];
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

        if (!status) {
            glGetShaderInfoLog(*shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    void init() {
        const bool hasGeometry = geometryPath != nullptr;

        // get shader code
        std::string vertexCode, fragmentCode, geometryCode;
        std::ifstream vShaderFile, fShaderFile, gShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            
            if(hasGeometry) {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        } catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }

        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // compile shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkError(&vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkError(&fragment, "FRAGMENT");

        if(hasGeometry) {
            const char *gShaderCode = geometryCode.c_str();

            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkError(&geometry, "GEOMETRY");
        }

        // create shader program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if(hasGeometry) glAttachShader(ID, geometry);

        GLint status;
        char infoLog[512];
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &status);

        if (!status) {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(hasGeometry) glDeleteShader(geometry);

        inited = true;
    }

public:
    unsigned int ID;

    Shader(
        const GLchar *vertexPath,
        const GLchar *fragmentPath,
        const GLchar *geometryPath = nullptr
    ) {
        this->vertexPath = vertexPath;
        this->fragmentPath = fragmentPath;
        this->geometryPath = geometryPath;
    }

    ~Shader() { glDeleteProgram(ID); }

    void use() {
        if (!inited) init();
        glUseProgram(ID);
    }

    void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
    void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
    void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
    void set2f(const std::string &name, float v1, float v2) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), v1, v2); }
    void set3f(const std::string &name, float v1, float v2, float v3) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3); }
    void set4f(const std::string &name, float v1, float v2, float v3, float v4) const { glUniform4f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3, v4); }
    void setVec2(const std::string &name, const glm::vec2 &value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
    void setVec3(const std::string &name, const glm::vec3 &value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
    void setMat4(const std::string &name, const glm::mat4 matrix) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix)); }
};

#endif
