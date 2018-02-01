#ifndef __AP_FFT__
#define __AP_FFT__

#include <vector>
#include "AudioFFT.h"

namespace ap {

struct FFT : audiofft::AudioFFT {
  unsigned size = 0;
  std::vector<float> magnitude, phase;

  void setup(unsigned size) {
    this->size = size;
    init(size);
    magnitude.resize(audiofft::AudioFFT::ComplexSize(size), 0);
    phase.resize(audiofft::AudioFFT::ComplexSize(size), 0);
  }

  void forward(const float* data) {
    // make aliases for the magnitude and phase vectors
    float* real = &magnitude[0];
    float* imag = &phase[0];

    // compute the FFT
    fft(data, real, imag);

    // convert to (magnitude, phase) representation
    for (unsigned i = 0; i < phase.size(); ++i) {
      float m = sqrt(real[i] * real[i] + imag[i] * imag[i]);
      // XXX is this the best way to compute phase?
      float p = (real[i] == 0.0f) ? M_PI / 2 : atan(imag[i] / real[i]);
      // it is very important that the next lines be separate and after the
      // previous lines because we are modifying data "in place".
      magnitude[i] = m;
      phase[i] = p;
    }
  }

  void reverse(float* data) {
    // make aliases for the magnitude and phase vectors
    float* real = &magnitude[0];
    float* imag = &phase[0];

    // convert to (real, imaginary) representation
    for (unsigned k = 0; k < phase.size(); ++k) {
      float r = magnitude[k] * cos(phase[k]);
      float i = magnitude[k] * sin(phase[k]);
      // it is very important that the next lines be separate and after the
      // previous lines because we are modifying data "in place".
      real[k] = r;
      imag[k] = i;
    }

    // compute the FFT
    ifft(data, real, imag);
  }
};

}  // namespace ap

#endif
