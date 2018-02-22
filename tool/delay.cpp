#include <cmath>
#include <iostream>
#include "AudioPlatform/Globals.h"  // die
#include "AudioPlatform/Types.h"    // ap::Array
using namespace std;

using namespace ap;

struct Delay : Array {
  float delay;
  unsigned next;

  Delay(float size = 2) {
    resize(sampleRate * size);
    next = 0;
  }

  void period(float s) { delay = s * sampleRate; }
  void ms(float ms) { period(ms / 1000); }
  void frequency(float f) { period(1 / f); }

  float effectValue(float v) {
    float index = next - delay;
    if (index < 0) index += size;
    float returnValue = get(index);
    data[next] = v;
    next++;
    return returnValue;
  }
  float operator()(float v) { return effectValue(v); };
};

int main() {
  Delay delay;
  delay.frequency(440);
  float f = 1;
  for (unsigned i = 0; i < 1000; ++i) {
    f = delay(f) / 2 + f / 2;
    printf("%f\n", f);
  }
}
