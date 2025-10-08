#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "perlin.hpp" // simple 2D Perlin noise with fbm wrapper
#include <utility>

class HeightmapGenerator {
public:
    struct DefaultParams {
        static constexpr int OCTAVES     = 8;
        static constexpr float FREQUENCY = 0.006f;
        static constexpr float AMPLITUDE = 4.0f;
        static constexpr float GAIN      = 0.5f;
        static constexpr float LACUNARITY= 1.1f;
    };

    HeightmapGenerator(int width, int depth)
        : width(width), depth(depth)
    {
        setParameters(DefaultParams::OCTAVES, DefaultParams::FREQUENCY,
                      DefaultParams::AMPLITUDE, DefaultParams::GAIN,
                      DefaultParams::LACUNARITY);

        heights.resize(width * depth, 0.0f);
        slopes.resize(width * depth, 0.0f);
    }

    // Set base procedural parameters (applied to base layer)
    void setParameters(int oct, float freq, float amp, float g, float lac) {
        octaves   = oct;
        frequency = freq;
        amplitude = amp;
        gain      = g;
        lacunarity= lac;
    }

    // Regenerate base heightmap + persistent layers
    void regenerate() {
        // base layer
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                float nx = x * frequency;
                float nz = z * frequency;
                float h = perlin::fbm2d(nx, nz, octaves, lacunarity, gain) * amplitude;
                heights[z * width + x] = h;
            }
        }

        // apply all extra persistent layers
        for (const auto &layer : extraNoiseLayers) {
            float layerFreq = layer.first;
            float layerAmp  = layer.second;
            for (int z = 0; z < depth; ++z) {
                for (int x = 0; x < width; ++x) {
                    float nx = x * layerFreq;
                    float nz = z * layerFreq;
                    float h = perlin::fbm2d(nx, nz, octaves, lacunarity, gain) * layerAmp;
                    heights[z * width + x] += h;
                }
            }
        }

        computeSlopes();
    }

    void applyThermalErosion(int iterations = 1, float repose = 1.0f, float talusFactor = 0.25f) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<float> deltaH(width * depth, 0.0f);

        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                float hCenter = heights[z * width + x];

                // Find neighbor with largest difference
                float maxDelta = 0.0f;
                int nxMax = -1, nzMax = -1;

                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dz == 0) continue;
                        int nx = x + dx;
                        int nz = z + dz;
                        if (nx < 0 || nx >= width || nz < 0 || nz >= depth) continue;

                        float hNeighbor = heights[nz * width + nx];
                        float diff = hCenter - hNeighbor;
                        if (diff > maxDelta) {
                            maxDelta = diff;
                            nxMax = nx;
                            nzMax = nz;
                        }
                    }
                }

                if (maxDelta > repose && nxMax != -1) {
                    float move = (maxDelta - repose) * talusFactor;
                    deltaH[z * width + x] -= move;
                    deltaH[nzMax * width + nxMax] += move;
                }
            }
        }

        // Apply changes
        for (int i = 0; i < heights.size(); ++i) {
            heights[i] += deltaH[i];
        }
    }

    computeSlopes();
}


    // Persistent layer management
    void addLayer(float freq, float amp) {
        extraNoiseLayers.emplace_back(freq, amp);
    }

    void removeLayer(size_t index) {
        if (index < extraNoiseLayers.size())
            extraNoiseLayers.erase(extraNoiseLayers.begin() + index);
    }

    void clearLayers() {
        extraNoiseLayers.clear();
    }

    // Query
    float getHeight(int x, int z) const {
        if (x < 0 || x >= width || z < 0 || z >= depth) return 0.0f;
        return heights[z * width + x];
    }

    float getSlope(int x, int z) const {
        if (x < 0 || x >= width || z < 0 || z >= depth) return 0.0f;
        return slopes[z * width + x];
    }

    int getWidth() const { return width; }
    int getDepth() const { return depth; }

    const std::vector<float>& getHeights() const { return heights; }
    std::vector<std::pair<float,float>>& getLayers() { return extraNoiseLayers; }

private:
    int width, depth;
    int octaves;
    float frequency, amplitude, gain, lacunarity;

    std::vector<float> heights;
    std::vector<float> slopes;

    // persistent layers (frequency, amplitude)
    std::vector<std::pair<float, float>> extraNoiseLayers;

    void computeSlopes() {
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                float hL = getHeight(x - 1, z);
                float hR = getHeight(x + 1, z);
                float hD = getHeight(x, z - 1);
                float hU = getHeight(x, z + 1);

                float dx = (hR - hL) * 0.5f;
                float dz = (hU - hD) * 0.5f;

                slopes[z * width + x] = glm::length(glm::vec2(dx, dz));
            }
        }
    }
};

