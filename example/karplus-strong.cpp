#include <cmath>
#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct Delay : Array {
  float ago;
  unsigned next;
  Delay(float capacity = 2) {
    resize(ceil(capacity * sampleRate));
    next = 0;
  }

  void period(float s) { ago = s * sampleRate; }
  void ms(float ms) { period(ms / 1000); }
  void frequency(float f) { period(1 / f); }

  float effectValue(float sample) {
    float index = next - ago;
    if (index < 0) index += size;
    float returnValue = get(index);
    data[next] = sample;
    next++;
    if (next >= size) next = 0;
    return returnValue;
  }
  float operator()(float sample) { return effectValue(sample); }
};

float r(float low, float high) {
  return low + (high - low) * rand() / RAND_MAX;
}

struct App : AudioVisual {
  SoundDisplay soundDisplay;
  Noise noise;
  Delay delay;
  Biquad filter;
  Biquad dcblock;

  Timer timer;

  Line envelope, gain, feedback;
  Line filterFrequency, delayFrequency;

  void setup() {
    timer.ms(600);

    dcblock.hpf(30, 0.7);

    soundDisplay.setup(4 * blockSize);
  }

  void audio(float* out) {
    static float f = 0;
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      if (timer()) envelope.set(1, 0, 150);

      filter.lpf(filterFrequency(), 0.1);

      f = dcblock(filter(delay(noise() * envelope() + feedback() * f / 2)) +
                  f / 2);

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
      static float db = -5.0f;
      ImGui::SliderFloat("Level (dB)", &db, -40.0f, 3.0f);
      gain.set(dbtoa(db));

      static float d = 35;
      ImGui::SliderFloat("Delay (Hz)", &d, 0, 127);
      delay.frequency(mtof(d));

      static float v = 130;
      ImGui::SliderFloat("Filter (Hz)", &v, 100, 134);
      filterFrequency.set(mtof(v));

      static float f = -3;
      ImGui::SliderFloat("Feedback (dB)", &f, -6, 0);
      feedback.set(dbtoa(f));

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
