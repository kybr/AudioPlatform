#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

float mtof(float m) { return 8.175799f * pow(2.0f, m / 12.0f); }
float ftom(float f) { return 12.0f * log2(f / 8.175799f); }

int main() {
  float sampleRate = 44100;
  float midi = 127;
  float phase = 0;

  while (midi > 0) {
    float frequency = mtof(midi);
    // printf("%f ", frequency);

    unsigned highestHarmonic = 1;
    while (highestHarmonic * frequency < sampleRate / 2) highestHarmonic++;
    highestHarmonic--;

    // printf("%u ", highestHarmonic);

    float distance = ftom(sampleRate / 2) - ftom(highestHarmonic * frequency);
    // printf("%f ", distance);

    distance /= 10;
    if (distance > 1) distance = 1;

    // for each harmonic
    float sum = distance * sin(highestHarmonic * phase);
    for (unsigned h = highestHarmonic - 1; h > 0; h--) sum += sin(h * phase);
    // float sum = 0.0f;
    // for (unsigned h = highestHarmonic; h > 0; h--) sum += sin(h * phase);

    // divide out by the number of harmonics because they were all amplitude 1
    sum /= highestHarmonic;

    // spit out the result
    printf("%f\n", sum);

    // move the phase forward
    phase += 2 * M_PI * frequency / sampleRate / 2;

    // totally arbitrary; no meaning; just tuned to be about right
    midi -= 0.001;
  }
}
