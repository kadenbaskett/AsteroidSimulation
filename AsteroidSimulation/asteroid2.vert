#version 330 core
layout (location = 0) in vec3 pos;

out vec2 TexCoords;

uniform mat4 mvp;

void main()
{
    vec4 P = mvp * vec4(pos, 1.0);
    TexCoords = (P.xy + vec2(1, 1)) / 2.0;
    gl_Position = P;
} 