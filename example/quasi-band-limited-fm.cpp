#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

struct QuasiBandlimited {
  //
  // "Synthesis of Quasi-Bandlimited Analog Waveforms Using Frequency
  // Modulation"
  //   by Peter Schoffhauzer
  // (http://scp.web.elte.hu/papers/synthesis1.pdf)
  //
  const float a0 = 2.5;   // precalculated coeffs
  const float a1 = -1.5;  // for HF compensation

  // variables
  float osc;      // output of the saw oscillator
  float osc2;     // output of the saw oscillator 2
  float phase;    // phase accumulator
  float w;        // normalized frequency
  float scaling;  // scaling amount
  float DC;       // DC compensation
  float norm;     // normalization amount
  float last;     // delay for the HF filter

  float Frequency, Filter, PulseWidth;

  QuasiBandlimited() {
    reset();
    Frequency = 1.0;
    Filter = 1.0;
    PulseWidth = 0.5;
    recalculate();
  }

  void reset() {
    // zero oscillator and phase
    osc = 0.0;
    osc2 = 0.0;
    phase = 0.0;
  }

  void recalculate() {
    w = Frequency / ap::sampleRate;  // normalized frequency
    float n = 0.5 - w;
    scaling = Filter * 13.0f * powf(n, 4.0f);  // calculate scaling
    DC = 0.376 - w * 0.752;                    // calculate DC compensation
    norm = 1.0 - 2.0 * w;                      // calculate normalization
  }

  void frequency(float f) {
    Frequency = f;
    recalculate();
  }

  void filter(float f) {
    Filter = f;
    recalculate();
  }

  void pulseWidth(float w) {
    PulseWidth = w;
    recalculate();
  }

  void step() {
    // increment accumulator
    phase += 2.0 * w;
    if (phase >= 1.0) phase -= 2.0;
    if (phase <= -1.0) phase += 2.0;
  }

  // process loop for creating a bandlimited saw wave
  float saw() {
    step();

    // calculate next sample
    osc = (osc + sinf(2 * M_PI * (phase + osc * scaling))) * 0.5;
    // compensate HF rolloff
    float out = a0 * osc + a1 * last;
    last = osc;
    out = out + DC;     // compensate DC offset
    return out * norm;  // store normalized result
  }

  // process loop for creating a bandlimited PWM pulse
  float pulse() {
    step();

    // calculate saw1
    osc = (osc + sinf(2 * M_PI * (phase + osc * scaling))) * 0.5;
    // calculate saw2
    osc2 =
        (osc2 + sinf(2 * M_PI * (phase + osc2 * scaling + PulseWidth))) * 0.5;
    float out = osc - osc2;  // subtract two saw waves
    // compensate HF rolloff
    out = a0 * out + a1 * last;
    last = osc;
    return out * norm;  // store normalized result
  }
};

// the AudioPlatform framework now uses the namespace "ap"
using namespace ap;

struct App : AudioVisual {
  SoundDisplay soundDisplay;

  QuasiBandlimited quasi;
  Line gain;
  Line frequency;
  Line filter;

  void setup() { soundDisplay.setup(4 * blockSize); }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      // set the frequency of the sine oscillator using the Line class to smooth
      // out jumps control values
      quasi.filter(filter());
      quasi.frequency(frequency());

      // compute the next sample
      float f = quasi.saw() * gain();

      // copy the sample to the right and left output channels
      out[i + 1] = out[i + 0] = f;

      soundDisplay(f);
    }
  }

  void visual() {
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

    // make a slider for note value (frequency)
    static float fil = 0.1;
    ImGui::SliderFloat("\"Filter\"", &fil, 0, 1);
    filter.set(fil, 50.0f);

    soundDisplay();

    ImGui::End();
  }
};

int main() { App().start(); }
