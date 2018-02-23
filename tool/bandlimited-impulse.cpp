#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

double mtof(double m) { return 8.175799 * pow(2.0, m / 12.0); }
double ftom(double f) { return 12.0 * log2(f / 8.175799); }

int main() {
  double sampleRate = 44100;
  double midi = ftom(22050);
  double phase = 0;

  unsigned _highestHarmonic = 1;

  while (midi > 0) {  // ftom(0.999999)) {
    double frequency = mtof(midi);

    unsigned highestHarmonic = 1;
    while (highestHarmonic * frequency < sampleRate / 2) highestHarmonic++;
    highestHarmonic--;

    if (highestHarmonic != _highestHarmonic) {
    }

    //  printf("f:%f ", frequency);
    //  printf("h:%u ", highestHarmonic);

    // for each harmonic
    double sum = 0;
    for (unsigned h = highestHarmonic; h > 0; h--) sum += sin(h * phase);

    // divide out by the number of harmonics because they were all amplitude 1
    sum /= (highestHarmonic + 0.44);
    //    sum *= gain;

    // spit out the result
    printf("%f\n", sum);
    // printf("s:%f\n", sum);

    // move the phase forward
    phase += 2 * M_PI * frequency / sampleRate / 2;

    // totally arbitrary; no meaning; just tuned to be about right
    midi -= 0.0003;

    _highestHarmonic = highestHarmonic;
  }
}
