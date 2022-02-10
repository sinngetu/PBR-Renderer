#ifndef MATERIAL_H
#define MATERIAL_H

#include <renderer/shader.h>

class Material {
protected:
    Shader *shader;

public:
    Material(Shader &shader) {
        this->shader = &shader;
    }

    Material(Shader *shader) {
        this->shader = shader;
    }

    void use() {
        shader->use();
    }

    void setM(glm::mat4 M) {
        use();
        shader->setMat4("model", M);
    }

    void setVP(glm::mat4 V, glm::mat4 P) {
        use();
        shader->setMat4("view", V);
        shader->setMat4("projection", P);
    }

    void setViewPositon(glm::vec3 pos) {
        use();
        shader->setVec3("viewPosition", pos);
    }
};
#endif
