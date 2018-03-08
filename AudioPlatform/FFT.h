#ifndef __AP_FFT__
#define __AP_FFT__

#include <functional>
#include <vector>
#include "AudioFFT/AudioFFT.h"

namespace ap {

struct FFT : audiofft::AudioFFT {
  void setup(unsigned size);
  void forward(const float* data);
  void reverse(float* data);

  unsigned size = 0;
  std::vector<float> magnitude, phase;
};

struct STFT : FFT {
  struct Buf : std::vector<float> {
    unsigned n;
  } in[2], out[2];
  std::vector<float> window;
  unsigned windowSize, hop;

  void setup(unsigned _windowSize);
  bool operator()(float f);
  float operator()();
  float operator()(float f, std::function<void(void)> process);
};

}  // namespace ap

#endif
