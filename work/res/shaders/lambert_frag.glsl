#version 330 core

in vec3 vNormal;
in vec2 vTexCoord;
in float vHeight; // world-space height passed from vertex shader

out vec4 fragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;

uniform sampler2D uTexGrass;
uniform sampler2D uTexStone;
uniform sampler2D uTexSnow;

uniform vec3 uTintGrass;
uniform vec3 uTintStone;
uniform vec3 uTintSnow;

uniform float uMinHeight;
uniform float uMaxHeight;
uniform float uTileX;
uniform float uTileZ;

void main() {
    // lighting
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 lighting = uAmbientColor + diff * uLightColor;

    // normalized height [0,1]
    float hNorm = clamp((vHeight - uMinHeight) / (uMaxHeight - uMinHeight), 0.0, 1.0);

    // tiled UVs
    vec2 tiledUV = vec2(vTexCoord.x * uTileX, vTexCoord.y * uTileZ);

    // sample textures
    vec3 grassCol = texture(uTexGrass, tiledUV).rgb * uTintGrass;
    vec3 stoneCol = texture(uTexStone, tiledUV).rgb * uTintStone;
    vec3 snowCol  = texture(uTexSnow,  tiledUV).rgb * uTintSnow;

    // thresholds
    float grassTop = 0.2;
    float stoneTop = 0.6;
    float blend = 0.2; // narrow blending region

    // weights with narrow smoothstep blending
    float wGrass = smoothstep(0.0, grassTop, hNorm) * (1.0 - smoothstep(grassTop, grassTop + blend, hNorm));
    float wStone = smoothstep(grassTop, stoneTop, hNorm) * (1.0 - smoothstep(stoneTop, stoneTop + blend, hNorm));
    float wSnow  = smoothstep(stoneTop, 1.0, hNorm);

    // normalize to sum to 1
    float sum = wGrass + wStone + wSnow;
    wGrass /= sum;
    wStone /= sum;
    wSnow  /= sum;

    // final color
    vec3 color = wGrass * grassCol + wStone * stoneCol + wSnow * snowCol;
    fragColor = vec4(color * lighting, 1.0);
}

