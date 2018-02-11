#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Synths.h"

using namespace ap;
using namespace std;

void findPeak(vector<float>& signal, vector<unsigned>& peak) {
  for (unsigned i = 1; i < signal.size(); ++i)
    if (signal[i - 1] < signal[i])
      if (signal[i + 1] < signal[i]) peak.push_back(i);
}

int main() {
  auto nextPowerOfTwo = [](float x) {
    return unsigned(pow(2, ceil(log(x) / log(2))));
  };
  SamplePlayer player;
  player.load("media/TingTing.wav");
  float sampleRate = player.playbackRate;
  unsigned windowSize = nextPowerOfTwo(0.01 * sampleRate);
  FFT fft;
  fft.setup(windowSize);

  struct Data {
    unsigned frame;
    float frequency, magnitude;
  };
  vector<Data> data;
  float maximum = 0;
  for (unsigned frame = 0; frame < 2 * (player.size / windowSize - 1);
       ++frame) {
    fft.forward(&player[frame * windowSize / 2]);
    vector<unsigned> peak;
    findPeak(fft.magnitude, peak);

    sort(peak.begin(), peak.end(), [&](unsigned a, unsigned b) {
      return fft.magnitude[a] > fft.magnitude[b];
    });
    for (unsigned i = 0; i < 16; ++i) {
      float m = fft.magnitude[peak[i]];
      if (maximum < m) maximum = m;
      data.push_back(
          {frame, peak[i] * sampleRate / 2 / fft.magnitude.size(), m});
    }
  }
  unsigned frame = 0;
  for (auto& d : data) {
    if (d.frame == frame)
      printf("%f:%f ", d.frequency, d.magnitude / maximum);
    else {
      printf("\n%f:%f ", d.frequency, d.magnitude / maximum);
      frame = d.frame;
    }
  }
  printf("\n");
}
