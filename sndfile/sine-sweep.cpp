#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int main() {
  int N = 88200;
  for (int i = 0; i < N; ++i) {
    float frequency = mtof(135.0f * i / N);
    // printf("f:%f\n", frequency);
    printf("%f\n", 0.707 * sin(frequency * i * M_PI * 2 / N));
  }
}
