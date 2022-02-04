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

    void use() {
        shader->use();
    }
};
#endif
