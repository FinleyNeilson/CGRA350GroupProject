	#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "perlin.hpp" // simple 2D Perlin noise with fbm wrapper

class HeightmapGenerator {
public:
    HeightmapGenerator(int width, int depth)
        : width(width), depth(depth),
          octaves(5), frequency(0.01f), amplitude(1.0f), gain(0.5f), lacunarity(2.0f)
    {
        heights.resize(width * depth, 0.0f);
        slopes.resize(width * depth, 0.0f);
    }

    // Set procedural parameters
    void setParameters(int oct, float freq, float amp, float g, float lac) {
        octaves = oct;
        frequency = freq;
        amplitude = amp;
        gain = g;
        lacunarity = lac;
    }

    // Regenerate the heightmap
    void regenerate() {
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                float nx = x * frequency;
                float nz = z * frequency;

                // 2D Perlin fBm
                float h = perlin::fbm2d(nx, nz, octaves, lacunarity, gain);
                h *= amplitude;

                heights[z * width + x] = h;
            }
        }

        computeSlopes();
    }

    void addCoarseNoise(float frequency, float amplitude, int octaves = 6, float lacunarity = 1.2f, float gain = 0.5f) {
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            float nx = x * frequency;
            float nz = z * frequency;

            float h = perlin::fbm2d(nx, nz, octaves, lacunarity, gain) * amplitude;
            heights[z*width + x] += h;
        }
    }
    computeSlopes();
}

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

private:
    int width, depth;
    int octaves;
    float frequency, amplitude, gain, lacunarity;

    std::vector<float> heights;
    std::vector<float> slopes;

    // Simple slope approximation
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
