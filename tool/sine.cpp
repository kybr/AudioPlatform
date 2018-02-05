#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>
int main() {
  for (int i = 0; i < 44100; ++i) {
    printf("%f\n", 0.707 * sin(440.0f * (float)i * M_PI * 2 / 44100));
  }
}
