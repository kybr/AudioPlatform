#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
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
  const unsigned historySize = 4 * blockSize;

  FFT fft;

  SawMinBLEP saw;
  Line gain;
  Line frequency;
  Line playerFrequency;

  std::mutex m;
  std::vector<float> history, _history;

  void setup() override {
    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    fft.setup(historySize);
    saw.load("media/MinBLEP.wav");
  }

  void audio(float* out) override {
    // "static" variables are scoped to the block (in this case the function)
    // and their value is persistent. statics are also used in the GUI blocks
    // later.
    static unsigned n = 0;

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
      _history[n] = f;
      n++;
    }

    // if the history buffer is full...
    if (n >= historySize) {
      // start a new history
      n = 0;

      // *maybe* copy the history buffer for the visual thread to use
      if (m.try_lock()) {
        memcpy(&history[0], &_history[0], historySize * sizeof(float));
        m.unlock();
      }
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

    // get the lock (mutex); this will block, waiting for the audio thread to
    // release the lock. lock() waits while try_lock() does not.
    m.lock();

    // use the history buffer to draw the waveform
    ImGui::PlotLines("Waveform", &history[0], historySize, 0, "", FLT_MAX,
                     FLT_MAX, ImVec2(0, 50));

    // take the FFT
    fft.forward(&history[0]);

    // release the lock; this is important. if you don't release the lock,
    // then the audio thread can never get the lock, so it can never copy any
    // history and we'll stop getting updates. we can release the lock as soon
    // as we are done using the history vector
    m.unlock();

    // convert to dB scale on the y axis
    for (auto& f : fft.magnitude) f = atodb(f);

    // draw the spectrum, linear in frequency
    ImGui::PlotLines("Spectrum", &fft.magnitude[0], fft.magnitude.size(), 0, "",
                     FLT_MAX, FLT_MAX, ImVec2(0, 50));

    // draw the spectrum, LOG in frequency
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // these are C++ "lambda" functions
    auto fix = [](float& f, float scale, float offset) {
      f = offset + f * scale;
    };
    auto flip = [](float& f, float level) { f = level - f; };

    auto line = [&](float x0, float y0, float x1, float y1) {
      // these numbers are magic and brittle :( but for now they work
      fix(x0, 7, 10);
      fix(x1, 7, 10);
      flip(y0, 100);
      flip(y1, 100);
      fix(y0, 4, 50);
      fix(y1, 4, 50);
      drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), ImColor(255, 255, 255));
    };

    // draw the spectrum with lines
    float _n = ftom(sampleRate * 1 / fft.magnitude.size() / 2);
    for (unsigned i = 2; i < fft.magnitude.size(); ++i) {
      float n = ftom(sampleRate * i / fft.magnitude.size() / 2);
      line(_n, fft.magnitude[i - 1], n, fft.magnitude[i]);
      _n = n;
    }

    ImGui::End();
  }
};

int main() { App().start(); }
