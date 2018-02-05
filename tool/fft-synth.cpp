#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>
#include <vector>
#include "AudioPlatform/FFT.h"

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int windowSize = 65536;  // most FFT implementations require a power of 2
float sampleRate = 44100;
float nyquistFrequency = sampleRate / 2;

void set(ap::FFT& fft, float frequency, float magnitude) {
  float normalizedFrequency = frequency / nyquistFrequency;
  unsigned nyquistIndex = fft.magnitude.size() - 1;

  // truncation of data next!
  unsigned closestIndex = normalizedFrequency * nyquistIndex;

  // synthesize
  fft.magnitude[closestIndex] = magnitude * fft.magnitude.size();
  fft.phase[closestIndex] = M_PI / 2;
}

int main() {
  ap::FFT fft;
  fft.setup(windowSize);
  // at this point fft.magnitude and fft.phase are zeros

  std::vector<float> output;
  output.resize(windowSize, 0);

  float frequency = mtof(60);  // middle-C is 60

  set(fft, frequency, 1);
  for (int h = 2; h < 23; h++) set(fft, frequency * h, 1.0f / h);

  fft.reverse(&output[0]);

  for (float f : output) printf("%f\n", f);
}
