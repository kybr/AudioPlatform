#include "AudioPlatform/FFT.h"
#include <cmath>

namespace ap {

void FFT::setup(unsigned size) {
  this->size = size;
  init(size);
  magnitude.resize(audiofft::AudioFFT::ComplexSize(size), 0);
  phase.resize(audiofft::AudioFFT::ComplexSize(size), 0);
}

void FFT::forward(const float* data) {
  // make aliases for the magnitude and phase vectors
  float* real = &magnitude[0];
  float* imag = &phase[0];

  // compute the FFT
  fft(data, real, imag);

  // convert to (magnitude, phase) representation
  for (unsigned i = 0; i < phase.size(); ++i) {
    float m = sqrt(real[i] * real[i] + imag[i] * imag[i]);

    // XXX this (below) is not good enough...
    //  float p = (real[i] == 0.0f) ? M_PI / 2 : atan(imag[i] / real[i]);

    // calculating phase is this complicated...
    // https://stackoverflow.com/questions/9453714/converting-real-and-imaginary-fft-output-to-frequency-and-amplitude#9454906
    float p = 0;
    if (real[i] == 0.0f)
      if (imag[i] > 0.0f)
        // pi/2 on the positive y-axis
        p = M_PI / 2;
      else
        // -pi/2 on the negative y-axis
        p = -M_PI / 2;
    else if (real[i] > 0)
      // arctan(b/a) in quadrants I and IV
      p = atan(imag[i] / real[i]);
    else if (imag[i] > 0.0f)
      // arctan(b/a) + pi in quadrant II
      p = atan(imag[i] / real[i]) + M_PI;
    else
      // arctan(b/a) - pi in quadrant III
      p = atan(imag[i] / real[i]) - M_PI;

    // it is very important that the next lines be separate and after the
    // previous lines because we are modifying data "in place".
    magnitude[i] = m;
    phase[i] = p;
  }
}

void FFT::reverse(float* data) {
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

}  // namespace ap
