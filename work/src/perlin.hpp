#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

namespace perlin {

inline float fade(float t) {
  // 6t^5 - 15t^4 + 10t^3 smoothstep function
  return t * t * t * (t * (t * 6 - 15) + 10);
}

inline float lerp(float a, float b, float t) { return a + t * (b - a); }

// Pseudorandom gradient at integer coordinates
inline std::pair<float, float> gradient(int ix, int iz) {
  static std::vector<std::pair<float, float>> grad2 = {
      {1, 0},  {-1, 0}, {0, 1},  {0, -1}, {1, 1},
      {-1, 1}, {1, -1}, {-1, -1}

  };
  unsigned int h = ix * 374761393 + iz * 668265263;  // some primes

  h = (h ^ (h >> 13)) * 1274126177;
  h = h ^ (h >> 16);
  return grad2[h % grad2.size()];
}

// 2D Perlin noise for a single point
inline float noise(float x, float z) {
  int x0 = static_cast<int>(floor(x));
  int z0 = static_cast<int>(floor(z));
  int x1 = x0 + 1;
  int z1 = z0 + 1;

  float sx = x - (float)x0;
  float sz = z - (float)z0;

  auto g00 = gradient(x0, z0);

  auto g10 = gradient(x1, z0);
  auto g01 = gradient(x0, z1);
  auto g11 = gradient(x1, z1);

  float dot00 = g00.first * (x - x0) + g00.second * (z - z0);
  float dot10 = g10.first * (x - x1) + g10.second * (z - z0);
  float dot01 = g01.first * (x - x0) + g01.second * (z - z1);
  float dot11 = g11.first * (x - x1) + g11.second * (z - z1);

  float u = fade(sx);
  float v = fade(sz);

  float nx0 = lerp(dot00, dot10, u);
  float nx1 = lerp(dot01, dot11, u);
  float nxy = lerp(nx0, nx1, v);

  return nxy;
}

// Fractal Brownian Motion (fBm) for multiple octaves
inline float fbm2d(float x, float z, int octaves = 4, float lacunarity = 2.0f,
                   float gain = 0.5f) {
  float amplitude = 1.0f;
  float frequency = 1.0f;
  float sum = 0.0f;

  for (int i = 0; i < octaves; ++i) {
    sum += noise(x * frequency, z * frequency) * amplitude;
    amplitude *= gain;
    frequency *= lacunarity;
  }

  return sum;
}

}  // namespace perlin
