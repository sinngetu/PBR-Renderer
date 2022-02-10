#version 410 core

in vec2 uv;

out vec4 ST_Target;

uniform sampler2D theTexture;

void main() {
    float depth = texture(theTexture, uv).r;

    ST_Target = vec4(vec3(depth), 1.0);
}
