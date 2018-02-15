
#include <cmath>
#include <iostream>
using namespace std;

float sampleRate = 44100;

// F = ma // Newton's Second Law
// F = -kx // Hook's law
// F = -cv // damping

struct MassSpring {
  float position, velocity;
  float springFactor, dampingFactor;
  unsigned stepsPerSample;
  MassSpring() : position(0), velocity(0.03), stepsPerSample(10) {}

  void frequency(float f, float r = 0.002) {
    f /= (sampleRate / 2);
    f *= 2 * M_PI;
    f /= 2;  // this factor of 2 is mysterious :(

    // the natural frequency of *damped* harmonic oscillators is
    // not as straightforward as simple harmonic oscillators;
    // you have to do some math...
    float w0 = f / sqrt(1 - r * r);
    springFactor = w0 * w0;
    dampingFactor = r * 2 * w0;
  }

  void step() {
    // Euler's method
    velocity +=
        (-springFactor * position + -dampingFactor * velocity) / stepsPerSample;
    position += velocity / stepsPerSample;
  }

  float nextValue() {
    for (unsigned i = 0; i < stepsPerSample; ++i) step();
    return position;
  }
  float operator()() { return nextValue(); }
};

int main() {
  MassSpring oscillator;
  oscillator.frequency(220, 0.02);
  for (unsigned i = 0; i < 100000; ++i) printf("%f\n", oscillator());
}
