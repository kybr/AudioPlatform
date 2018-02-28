#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

//#define float double

float mtof(float m) { return 8.175799f * pow(2.0f, m / 12.0f); }

int main() {
  float sampleRate = 44100;
  float midi = 127;
  float phase = 0;

  while (midi > 0) {
    float frequency = mtof(midi);
    float increment = frequency / sampleRate / 2;
    increment *= M_PI * 2;

    float sum = 0.0f;
    float h = 1;
    while (h * frequency < sampleRate / 2) {
      sum += sin((double)h * phase);
      h++;
    }
    // printf("%f %f\n", frequency, h - 1);

    // divide out by the number of harmonics because they were all amplitude 1
    sum /= h;

    // spit out the result
    printf("%f\n", sum);

    // move the phase forward
    phase += increment;

    // totally arbitrary; no meaning; just tuned to be about right
    midi -= 0.001;
  }
}
