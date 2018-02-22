#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

/*
 Copy tool/additive-synth.cpp to mydir/bandlimited-impulse-sweep.cpp. Adapt
 this code to output a series of floats that will make a bandlimited impulse
 wave that sweeps from 127 (MIDIs) down to 0 (MIDIs). Your output should be
 longer than 5 seconds and 44100 Hz sample rate.
 */

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int main() {
  float sampleRate = 44100;
  float frequency = mtof(45);
  float increment = frequency / sampleRate / 2;
  printf("%f");
  increment *= 2 * M_PI;

  float phase = 0;
  while (phase < 44100) {
    float sum = 0.0f;

    sum += sin(phase);  // fundamental

    // harmonics
    for (int h = 2; h < 23; h++) sum += sin(h * phase) / h;

    // calculate amplitude
    float A = 1;
    for (int h = 2; h < 23; h++) A += 1.0f / h;

    printf("%f\n", 0.707 * sum / A);

    phase += increment;
  }
}
