#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Synths.h"

const float windowSeconds = 0.03;
const unsigned hopFactor = 8;

using namespace ap;
using namespace std;

void findPeak(vector<float>& signal, vector<unsigned>& peak) {
  for (unsigned i = 1; i < signal.size(); ++i)
    if (signal[i - 1] < signal[i])
      if (signal[i + 1] < signal[i]) peak.push_back(i);
}

int main(int argc, char* argv[]) {
  auto nextPowerOfTwo = [](float x) {
    return unsigned(pow(2, ceil(log(x) / log(2))));
  };
  SamplePlayer player;
  if (argc == 2) {
    player.load(argv[1]);
    fprintf(stderr, "loaded %s\n", argv[1]);
  } else
    player.load("media/TingTing.wav");
  float sampleRate = player.playbackRate;
  unsigned windowSize = nextPowerOfTwo(windowSeconds * sampleRate);

  Array window, copy;
  hanning(window, windowSize);
  copy.resize(windowSize);

  FFT fft;
  fft.setup(windowSize);

  struct Data {
    unsigned frame;
    float frequency, magnitude;
  };
  vector<Data> data;
  float maximum = 0;
  unsigned hop = windowSize / hopFactor;
  for (unsigned frame = 0; frame * hop < player.size - windowSize; ++frame) {
    //    printf("%u/%u %u\n", frame * hop, player.size, windowSize);

    memcpy(&copy[0], &player[frame * hop], sizeof(float) * windowSize);
    for (unsigned i = 0; i < windowSize; ++i) copy[i] *= window[i];
    fft.forward(&copy[0]);
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
  // return 0;
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
