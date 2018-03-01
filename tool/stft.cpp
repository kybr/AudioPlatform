#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct STFT : FFT {
  struct Buf : std::vector<float> {
    unsigned n;
  } in[2], out[2];
  std::vector<float> window;
  unsigned windowSize, hop;

  void setup(unsigned _windowSize) {
    windowSize = _windowSize;
    hop = windowSize / 2;
    FFT::setup(windowSize);
    hann(window, windowSize);
    for (auto& e : window) e = sqrt(e);

    //
    //
    in[0].resize(windowSize, 0);
    in[1].resize(windowSize, 0);
    out[0].resize(windowSize, 0);
    out[1].resize(windowSize, 0);

    // start out of phase
    //
    in[0].n = hop;
    in[1].n = 0;
    out[0].n = hop;
    out[1].n = 0;
  }

  bool operator()(float f) {
    // assume we did not do an FFT
    //
    bool returnValue = false;
    for (unsigned i = 0; i < 2; i++) {
      in[i].at(in[i].n) = f * window[in[i].n];
      in[i].n++;
      if (in[i].n >= in[i].size()) {
        // that f up there was the final sample we needed to
        // do our next FFT
        in[i].n = 0;
        FFT::forward(&in[i][0]);
        returnValue = true;
      }
    }
    return returnValue;
  }

  float operator()() {
    float sum = 0;
    for (unsigned i = 0; i < 2; i++) {
      if (out[i].n >= out[i].size()) out[i].n = 0;
      // if we're about to output the very first (0th) sample
      // of a block, then take the IFFT
      if (out[i].n == 0) FFT::reverse(&out[i][0]);
      sum += out[i].at(out[i].n) * window[out[i].n];
      out[i].n++;
    }
    return sum;
  }
};

int main() {
  STFT stft;
  stft.setup(1024);
  for (int i = 0; i < 5 * 1024; i++) {
    stft(1);
    printf("%f\n", stft());
  }
}
