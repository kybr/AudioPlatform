#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct App : AudioVisual {
  Sine sine;
  Line gain;
  Line frequency;

  Timer timer;
  void setup() { timer.ms(500); }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      if (timer()) {
        // do somthing every timer period
      }
      sine.frequency(frequency());
      float f = sine() * gain();
      out[i + 1] = out[i + 0] = f;
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
      static float note = 60;
      ImGui::SliderFloat("Frequency (MIDI)", &note, 0, 127);
      frequency.set(mtof(note), 50.0f);

      ImGui::End();
    }
  }
};

int main() { App().start(); }
