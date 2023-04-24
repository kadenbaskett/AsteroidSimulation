#version 330 core
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

uniform mat4 mvp;

void main()
{
    vec4 P = mvp * vec4(pos, 1.0);
    TexCoords = pos;
    gl_Position = P;
} 