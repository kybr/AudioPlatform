#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Types.h"

#include <cmath>

namespace ap {

float mtof(float m) { return 8.175799 * powf(2.0f, m / 12.0f); }
float ftom(float f) { return 12.0f * log2f(f / 8.175799); }
float dbtoa(float db) { return 1.0f * powf(10.0f, db / 20.0f); }
float atodb(float a) { return 20.0f * log10f(a / 1.0f); }

void hann(Array& window, unsigned size) {
  window.resize(size);
  for (unsigned i = 0; i < size; ++i)
    window[i] = (1 - cos(2 * M_PI * i / size)) / 2;
}

void hann(std::vector<float>& window, unsigned size) {
  window.resize(size);
  for (unsigned i = 0; i < size; ++i)
    window[i] = (1 - cos(2 * M_PI * i / size)) / 2;
}

void normalize(float* data, unsigned size) {
  float max = 0;
  for (unsigned i = 0; i < size; ++i)
    if (max > std::abs(data[i])) max = data[i];
  for (unsigned i = 0; i < size; ++i) data[i] /= max;
}

}  // namespace ap
