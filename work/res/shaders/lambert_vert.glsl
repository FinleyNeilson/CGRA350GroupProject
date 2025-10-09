#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 vNormal;
out vec2 vTexCoord;
out float vHeight; // world-space height

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

void main() {
    vHeight = aPosition.y;
    vTexCoord = aUV;
    vNormal = normalize(uNormalMatrix * aNormal);

    vec4 viewPos = uModelViewMatrix * vec4(aPosition, 1.0);
    gl_Position = uProjectionMatrix * viewPos;
}
