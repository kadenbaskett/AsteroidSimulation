#version 330 core
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

uniform mat4 mvp;

void main()
{
    TexCoords = pos;
    vec4 P = vec4(pos, 1.0);
    gl_Position = mvp * P;
} 