#include <cmath>
#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
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

void make_hann(std::vector<float>& window) {
  for (unsigned n = 0; n < window.size(); ++n)
    window[n] = (1 - cos(2 * M_PI * n / (window.size() - 1))) / 2;
}

float r(float low, float high) {
  return low + (high - low) * rand() / RAND_MAX;
}

struct App : AudioVisual {
  const unsigned historySize = 4 * blockSize;
  FFT fft;

  Noise noise;
  Delay delay;
  Biquad filter;
  Biquad dcblock;

  Timer timer;

  Line envelope, gain, feedback;
  Line filterFrequency, delayFrequency;

  std::mutex m;
  std::vector<float> history, _history;
  std::vector<float> hann;

  void setup() {
    timer.ms(600);

    dcblock.hpf(30, 0.7);

    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    hann.resize(historySize);
    make_hann(hann);
    fft.setup(historySize);
  }

  void audio(float* out) {
    static unsigned n = 0;
    static float f = 0;
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      if (timer()) envelope.set(1, 0, 150);

      filter.lpf(filterFrequency(), 0.1);

      f = dcblock(filter(delay(noise() * envelope() + feedback() * f / 2)) +
                  f / 2);

      out[i + 1] = out[i + 0] = f * gain();
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

      // get the lock (mutex); this will block, waiting for the audio thread to
      // release the lock. lock() waits while try_lock() does not.
      m.lock();

      // use the history buffer to draw the waveform
      ImGui::PlotLines("Waveform", &history[0], historySize, 0, "", FLT_MAX,
                       FLT_MAX, ImVec2(0, 50));

      // make a copy
      float copy[historySize];
      memcpy(copy, &history[0], historySize * sizeof(float));
      // window the data
      for (unsigned i = 0; i < historySize; ++i) copy[i] *= hann[i];
      // take the FFT
      fft.forward(&copy[0]);

      // release the lock; this is important. if you don't release the lock,
      // then the audio thread can never get the lock, so it can never copy any
      // history and we'll stop getting updates. we can release the lock as soon
      // as we are done using the history vector
      m.unlock();

      // convert to dB scale on the y axis
      for (auto& f : fft.magnitude) f = atodb(f);

      // draw the spectrum, linear in frequency
      ImGui::PlotLines("Spectrum", &fft.magnitude[0], fft.magnitude.size(), 0,
                       "", FLT_MAX, FLT_MAX, ImVec2(0, 50));

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
        drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1),
                          ImColor(255, 255, 255));
      };

      // draw the spectrum with lines
      float _n = ftom(sampleRate * 1 / fft.magnitude.size() / 2);
      for (unsigned i = 2; i < fft.magnitude.size(); ++i) {
        float n = ftom(sampleRate * i / fft.magnitude.size() / 2);
        line(_n, fft.magnitude[i - 1], n, fft.magnitude[i]);
        _n = n;
      }
      ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x,
                  ImGui::GetIO().MousePos.y);
      ImGui::Text("Mouse State - 0:%d 1:%d 2:%d", ImGui::GetIO().MouseDown[0],
                  ImGui::GetIO().MouseDown[1], ImGui::GetIO().MouseDown[2]);

      ImGui::Text("Mouse Clicked - 0:%d 1:%d", ImGui::IsMouseClicked(0),
                  ImGui::IsMouseClicked(1));

      ImGui::End();
    }
  }
};

int main() { App().start(); }
