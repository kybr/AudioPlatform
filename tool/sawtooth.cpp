#include <cmath>  // std::sin, std::pow, M_PI
#include <iostream>
int main() {
  float y = -1;
  for (int i = 0; i < 44100; ++i) {
    printf("%f\n", y * 0.707);
    y += 2.0f / 44100 * 440;
    if (y > 1) y -= 2;
  }
}
