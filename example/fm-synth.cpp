#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct App : AudioVisual {
  SoundDisplay soundDisplay;
  Sine sine;
  Line gain;
  Line frequency;

  void setup() { soundDisplay.setup(4 * blockSize); }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      sine.frequency(frequency());
      float f = sine() * gain();
      out[i + 1] = out[i + 0] = f;
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
      static float note = 60;
      ImGui::SliderFloat("Frequency (MIDI)", &note, 0, 127);
      frequency.set(mtof(note), 50.0f);

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
