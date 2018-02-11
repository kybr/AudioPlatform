#include "AudioPlatform/Types.h"

#include <cmath>

namespace ap {

Array::~Array() {
  if (data) delete[] data;
}

float& Array::operator[](unsigned index) { return data[index]; }
float Array::operator[](const float index) const { return get(index); }

void Array::resize(unsigned n) {
  size = n;
  if (data) delete[] data;  // or your have a memory leak
  if (n == 0) {
    data = nullptr;
  } else {
    data = new float[n];
    for (unsigned i = 0; i < n; ++i) data[i] = 0.0f;
  }
}
void Array::zeros(unsigned n) { resize(n); }

float Array::get(const float index) const {
  const unsigned i = floor(index);
  const float x0 = data[i];
  const float x1 = data[(i == (size - 1)) ? 0 : i + 1];  // looping semantics
  const float t = index - i;
  return x1 * t + x0 * (1 - t);
}

void Array::add(const float index, const float value) {
  const unsigned i = floor(index);
  const unsigned j = (i == (size - 1)) ? 0 : i + 1;  // looping semantics
  const float t = index - i;
  data[i] += value * (1 - t);
  data[j] += value * t;
}
}  // namespace ap
