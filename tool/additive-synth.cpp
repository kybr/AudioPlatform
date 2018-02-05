#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>

float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }

int main() {
  int N = 88200;
  float frequency = mtof(45);
  float w = frequency * M_PI * 2 / N;
  for (int i = 0; i < N; ++i) {
    float sum = 0.0f;

    sum += sin(1.0 * w * i);  // fundamental

    for (int h = 2; h < 23; h++) sum += sin(h * w * i) / h;

    float A = 1;
    for (int h = 2; h < 23; h++) A += 1.0f / h;

    printf("%f\n", 0.707 * sum / A);
  }
}
