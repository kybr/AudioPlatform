#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct App : AudioVisual {
  SamplePlayer player;
  Line gain;
  Line frequency;
  SoundDisplay soundDisplay;

  void setup() {
    player.load("out.wav");
    // player.load("media/Impulse-Sweep.wav");
    soundDisplay.setup(4 * blockSize);
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      player.frequency(frequency());
      float f = player();
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
