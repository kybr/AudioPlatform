#ifndef __AP_FFT__
#define __AP_FFT__

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

}  // namespace ap

#endif
