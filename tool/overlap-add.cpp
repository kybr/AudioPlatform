#include <cmath>
#include <iostream>
#include <vector>
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Types.h"

int main() {
  ap::Array window, buffer;
  hann(window, 2048);
  buffer.resize(2048 * 20);
  for (unsigned i = 0; i < 2048 * 19; i += 1024)
    for (unsigned j = 0; j < 2048; ++j) buffer[i + j] += window[j];
  for (unsigned k = 0; k < buffer.size; ++k) printf("%f\n", buffer[k]);
}
