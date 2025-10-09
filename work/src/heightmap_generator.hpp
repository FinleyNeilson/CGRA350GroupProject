#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "perlin.hpp" // simple 2D Perlin noise with fbm wrapper
#include <utility>

class HeightmapGenerator {
public:
	struct DefaultParams {
		static constexpr int OCTAVES = 10;
		static constexpr float FREQUENCY = 0.003f;
		static constexpr float AMPLITUDE = 5.0f;
		static constexpr float GAIN = 0.35f;
		static constexpr float LACUNARITY = 1.8f;
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
		octaves = oct;
		frequency = freq;
		amplitude = amp;
		gain = g;
		lacunarity = lac;
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
		for (const auto& layer : extraNoiseLayers) {
			float layerFreq = layer.first;
			float layerAmp = layer.second;
			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					float nx = x * layerFreq;
					float nz = z * layerFreq;
					float h = perlin::noise(nx, nz) * layerAmp;
					heights[z * width + x] += h;
				}
			}
		}

		computeSlopes();
	}

	std::pair<float, float> computeMinMax() const {
		if (heights.empty()) return { 0.0f, 0.0f };
		float minH = heights[0], maxH = heights[0];
		for (float h : heights) {
			minH = std::min(minH, h);
			maxH = std::max(maxH, h);
		}
		return { minH, maxH };
	}

	float getHeightClamped(int x, int z) const {
		x = glm::clamp(x, 0, width - 1);
		z = glm::clamp(z, 0, depth - 1);
		return heights[z * width + x];
	}

	void applyThermalErosionMultiNeighbor(int iterations = 6,
		float reposeAngleDeg = 30.0f,
		float talusFactor = 0.15f,
		float cellSizeX = 0.02f,
		float cellSizeZ = 0.02f) {
		// compute recommended repose height difference across one cell
		// reposeHeight = tan(angle) * avgCellSpacing
		const float averageSpacing = (cellSizeX + cellSizeZ) * 0.5f;
		const float reposeRad = glm::radians(glm::clamp(reposeAngleDeg, 0.0f, 89.0f));
		const float reposeHeight = std::tan(reposeRad) * averageSpacing;

		// debug: print heights min/max and chosen reposeHeight
		auto mm = computeMinMax();
		std::cout << "Height range: min=" << mm.first << " max=" << mm.second
			<< " range=" << (mm.second - mm.first)
			<< " | reposeAngleDeg=" << reposeAngleDeg
			<< " => reposeHeight=" << reposeHeight << std::endl;

		talusFactor = glm::clamp(talusFactor, 0.0f, 1.0f);

		for (int iter = 0; iter < iterations; ++iter) {
			std::vector<float> deltaH(width * depth, 0.0f);

			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					const int idx = z * width + x;
					const float hC = heights[idx];

					// collect eligible lower neighbors
					float totalExcess = 0.0f;
					float excess[8];
					int nxArr[8], nzArr[8];
					int count = 0;

					for (int dz = -1; dz <= 1; ++dz) {
						for (int dx = -1; dx <= 1; ++dx) {
							if (dx == 0 && dz == 0) continue;
							int nx = x + dx;
							int nz = z + dz;
							if (nx < 0 || nx >= width || nz < 0 || nz >= depth) continue;

							float hN = heights[nz * width + nx];
							// horizontal distance to that neighbor:
							float hx = (dx == 0) ? 0.0f : cellSizeX;
							float hz = (dz == 0) ? 0.0f : cellSizeZ;
							// For diagonal neighbors, horizontal dist ~= sqrt(cellSizeX^2 + cellSizeZ^2)
							float horizDist = std::sqrt(hx * hx + hz * hz);
							if (horizDist <= 0.0f) horizDist = averageSpacing;

							// convert allowed slope (angle) to allowed height difference for this neighbor:
							float allowedDiff = std::tan(reposeRad) * horizDist;

							float diff = hC - hN;
							if (diff > allowedDiff) {
								float ex = diff - allowedDiff;
								excess[count] = ex;
								nxArr[count] = nx;
								nzArr[count] = nz;
								totalExcess += ex;
								++count;
							}
						}
					}

					if (count == 0 || totalExcess <= 0.0f) continue;

					float moveOut = totalExcess * talusFactor;
					for (int i = 0; i < count; ++i) {
						float share = excess[i] / totalExcess;
						float transfer = moveOut * share;
						deltaH[idx] -= transfer;
						deltaH[nzArr[i] * width + nxArr[i]] += transfer;
					}
				}
			}

			// apply changes
			for (size_t i = 0; i < heights.size(); ++i) heights[i] += deltaH[i];
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
	std::vector<std::pair<float, float>>& getLayers() { return extraNoiseLayers; }

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

