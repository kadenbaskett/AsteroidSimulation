#version 330 core
layout (location = 0) in vec3 pos;

out vec2 TexCoords;

uniform mat4 mvp;

uniform sampler2D asteroidDisplacement;

float displacementAmount = 0.75f;

void main()
{
    float displacement = texture(asteroidDisplacement, TexCoords).r;
    vec3 updatedPos = pos + normalize(pos) * displacement * displacementAmount;

    vec4 P = mvp * vec4(updatedPos, 1.0);
    TexCoords = (P.xy + vec2(1, 1)) / 2.0;
    gl_Position = P;
} 