#ifndef MATERIAL_H
#define MATERIAL_H

#include <renderer/shader.h>

class Material {
protected:
    Shader *shader;
    void (*setProps)(Shader *shader);

    void setPropsFunc(void (*function)(Shader *shader)) {
        setProps = function;
    }

public:
    Material(Shader &shader) {
        this->shader = &shader;
        setPropsFunc(nullptr);
    }

    Material(Shader &shader, void (*func)(Shader *shader)) {
        this->shader = &shader;
        setPropsFunc(func);
    }

    void use() {
        shader->use();

        if (setProps != nullptr)
            (*setProps)(shader);
    }
};
#endif
