#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec3 vPosition;

void main() {
    vec4 viewPos = uModelViewMatrix * vec4(aPosition, 1.0);
    vPosition = viewPos.xyz;

    vNormal = normalize(uNormalMatrix * aNormal);

    gl_Position = uProjectionMatrix * viewPos;
}
