#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; // (3 vertices) * (6 faces)

out vec4 direction;

uniform mat4 projection;
uniform mat4 views[6];

void main() {
    for(int face = 0; face < 6; face++) {
        gl_Layer = face; // switch the cubemap face

        for(int i = 0; i < 3; i++) {
            direction = gl_in[i].gl_Position;
            gl_Position =  projection * views[face] * direction;
            EmitVertex();
        }

        EndPrimitive();
    }
}
