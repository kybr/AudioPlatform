#include <memory>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
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
    // printf("in: n0=%u,n1=%u | ", in[0].n, in[1].n);
    bool returnValue = false;
    for (unsigned i = 0; i < 2; i++) {
      in[i].at(in[i].n) = f * window[in[i].n];
      in[i].n++;
      if (in[i].n >= in[i].size()) {
        in[i].n = 0;
        FFT::forward(&in[i][0]);  // compute the FFT
        returnValue = true;
        // return true;
      }
    }
    return returnValue;
  }

  float operator()() {
    // printf("out: n0=%u,n1=%u | size=%u\n", out[0].n, out[1].n, size);
    float sum = 0;
    for (unsigned i = 0; i < 2; i++) {
      if (out[i].n >= out[i].size()) out[i].n = 0;
      if (out[i].n == 0) FFT::reverse(&out[i][0]);  // compute the IFFT
      sum += out[i].at(out[i].n) * window[out[i].n];
      out[i].n++;
    }
    return sum;
  }

  /*
  float operator()() {
    // printf("out: n0=%u,n1=%u | size=%u\n", out[0].n, out[1].n, size);
    float sum = 0;
    for (unsigned i = 0; i < 2; i++) {
      sum += out[i].at(out[i].n) * window[out[i].n];
      out[i].n++;
      if (out[i].n >= out[i].size()) {
        out[i].n = 0;
        FFT::reverse(&out[i][0]);  // compute the IFFT
      }
    }
    return sum;
  }
  */
};

struct App : AudioVisual {
  Line frequency;
  Line gain;
  SamplePlayer player;
  SoundDisplay soundDisplay;
  STFT stft;

  void setup() {
    player.load("media/voice.wav");
    // player.load("media/TingTing.wav");
    // printf("got here\n");
    // player.load("media/Impulse-Sweep.wav");
    soundDisplay.setup(4 * blockSize);
    stft.setup(blockSize);
  }

  int bin = 20;

  void audio(float *out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      player.frequency(frequency());
      float f = player();
      if (stft(f)) {
        for (unsigned i = bin; i < stft.magnitude.size() - 1; ++i)
          stft.magnitude[i] = 0;
      }
      f = stft();
      out[i + 1] = out[i + 0] = f * gain();
      soundDisplay(f);
    }
  }

  void visual() {
    {
      // this stuff makes a single "root" window
      int windowWidth, windowHeight;
      glfwGetWindowSize(window, &windowWidth, &windowHeight);
      ImGui::SetWindowPos("window", ImVec2(0, 0));
      ImGui::SetWindowSize("window", ImVec2(windowWidth, windowWidth));
      ImGui::Begin("window", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoResize);

      // make a slider for "volume" level
      static float db = -60.0f;
      ImGui::SliderFloat("Level (dB)", &db, -60.0f, 3.0f);
      gain.set(dbtoa(db), 50.0f);

      ImGui::SliderInt("Bin", &bin, 0, stft.magnitude.size() - 1);

      // make a slider for note value (frequency)
      static float rate = 1.0;
      ImGui::SliderFloat("Rate", &rate, -1.0, 2.1);
      frequency.set(rate, 20.0f);

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
