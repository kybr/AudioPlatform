#include <cmath>
#include <iostream>
#include <vector>
#include "AudioPlatform/Audio.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Synths.h"

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int windowSize = 65536;
float sampleRate = 44100;
float nyquistFrequency = sampleRate / 2;

void set(ap::FloatArrayWithLinearInterpolation& magarr, float frequency,
         float magnitude) {
  float normalizedFrequency = frequency / nyquistFrequency;
  unsigned nyquistIndex = magarr.size - 1;

  // truncation of data next!
  // unsigned closestIndex = normalizedFrequency * nyquistIndex;
  float index = normalizedFrequency * nyquistIndex;

  // linearly interpolating array write
  magarr.set(index, magnitude * magarr.size);
}

int main() {
  ap::FFT fft;
  fft.setup(windowSize);
  // at this point fft.magnitude and fft.phase are zeros

  std::vector<float> output;
  output.resize(windowSize, 0);

  float frequency = mtof(60);  // middle-C is 60

  ap::FloatArrayWithLinearInterpolation arr;
  arr.zeros(fft.magnitude.size());
  // insert harmonics
  set(arr, frequency, 1);
  for (int h = 2; h < 23; h++) set(arr, frequency * h, 1.0f / h);
  // copy from float-index array to magnitude array
  for (unsigned i = 0; i < arr.size; ++i) fft.magnitude[i] = arr[i];

  fft.reverse(&output[0]);

  for (float f : output) printf("%f\n", f);
}
