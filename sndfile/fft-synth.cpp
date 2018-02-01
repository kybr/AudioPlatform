#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>
#include <vector>
#include "FFT.h"

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int main() {
  int windowSize = 65536;
  float sampleRate = 44100;
  float nyquistFrequency = sampleRate / 2;

  ap::FFT fft;
  fft.setup(windowSize);

  std::vector<float> output;
  output.resize(windowSize, 0);

  float frequency = mtof(60);  // middle-C is 60
  float normalizedFrequency = frequency / nyquistFrequency;
  unsigned nyquistIndex = fft.magnitude.size();
  // truncation of data next!
  unsigned closestIndex = normalizedFrequency * nyquistIndex;
  float goodMagnitude = fft.magnitude.size() / 2;

  // synthesize
  fft.magnitude[closestIndex] = goodMagnitude;
  fft.reverse(&output[0]);

  for (float f : output) printf("%f\n", f);
}
