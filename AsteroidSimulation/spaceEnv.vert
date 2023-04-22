#version 330 core
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = pos;
    gl_Position = projection * view * vec4(10 * pos.x, 10 * pos.y, 10 * pos.z, 1.0);
    // gl_Position = 10 * vec4(pos, 1.0);
} 