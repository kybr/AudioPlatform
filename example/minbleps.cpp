#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct SawMinBLEP : SamplePlayer {
  float phase = 0.0f, increment = 0.0f;
  void frequency(float hz) {
    increment = hz / sampleRate;
    // SamplePlayer::frequency(hz * 3);
  }

  void period(float s) { frequency(1 / s); }

  virtual float operator()() { return nextValue(); }

  virtual float nextValue() {
    //    float returnValue = phase;
    phase += increment;
    if (phase > 1.0f) {
      // phase wrapped; reset MinBLEP
      SamplePlayer::phase = 0;
      phase -= 1.0f;
    }
    if (phase < 0.0f) {
      // phase wrapped; reset MinBLEP
      SamplePlayer::phase = 0;
      phase += 1.0f;  // sure; handle negative frequency
    }
    return (2 * phase - 1) + (2 - (2 * SamplePlayer::nextValue()));
    // return (2 * returnValue - 1) + (2 - (2 * SamplePlayer::nextValue()));
  }
};

struct App : AudioVisual {
  SoundDisplay soundDisplay;

  SawMinBLEP saw;
  Line gain;
  Line frequency;
  Line playerFrequency;

  void setup() override {
    soundDisplay.setup(4 * blockSize);

    // this loaded sample is the result of...
    // - taking the bandlimited impulse into the cepstral domain
    // - zeroing the top 50% of the cepstral data
    // - bringing this back into the original real/time domain
    // - integrating
    // i think that process might be called "liftering"
    // https://en.wikipedia.org/wiki/Cepstrum#Liftering
    saw.load("media/MinBLEP.wav");
  }

  void audio(float* out) override {
    // for each pair of samples, left and right
    //
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      // set the frequency of the sine oscillator using the Line class to smooth
      // out jumps control values
      float v = frequency();
      saw.frequency(v);
      saw.SamplePlayer::frequency(v * playerFrequency());

      // compute the next sample
      float f = saw() * gain();

      // copy the sample to the right and left output channels
      out[i + 1] = out[i + 0] = f;

      // save each sample in to a history buffer
      soundDisplay(f);
    }
  }

  void visual() override {
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

    static float freq = 1;
    ImGui::SliderFloat("factor", &freq, 0, 5);
    playerFrequency.set(freq, 50.0f);

    soundDisplay();

    ImGui::End();
  }
};

int main() { App().start(); }
