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

struct App : AudioVisual {
  Line frequency;
  Line gain;
  SamplePlayer player;
  SoundDisplay soundDisplay;
  STFT stft;

  void setup() {
    // player.load("media/voice.wav");
    player.load("media/TingTing.wav");
    // player.load("media/Impulse-Sweep.wav");
    // player.load("media/sine.wav");
    soundDisplay.setup(4 * blockSize);
    stft.setup(blockSize * 2);
  }

  int bin = 20;

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      player.frequency(frequency());
      float f = player();
      if (stft(f)) {
        //
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
