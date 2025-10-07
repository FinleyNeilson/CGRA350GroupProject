#version 330 core

in vec3 vNormal;
in vec3 vPosition;

out vec4 fragColor;

uniform vec3 uColor;        
uniform vec3 uLightDir;     
uniform vec3 uLightColor;   
uniform vec3 uAmbientColor; 

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uLightColor * uColor;
    vec3 ambient = uAmbientColor * uColor;

    vec3 color = ambient + diffuse;
    fragColor = vec4(color, 1.0);
}

