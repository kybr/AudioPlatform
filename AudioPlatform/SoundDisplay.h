#ifndef __AP_SOUND_DISPLAY__
#define __AP_SOUND_DISPLAY__

#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/Globals.h"
#include "AudioPlatform/Types.h"

#include <string.h>
#include <mutex>
#include <vector>

namespace ap {

struct SoundDisplay {
  FFT fft;
  std::mutex m;
  std::vector<float> history, _history;
  Array hann_window;
  unsigned n = 0;

  void setup(unsigned historySize) {
    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    hann(hann_window, historySize);
    fft.setup(historySize);
  }

  void operator()(float value) {
    _history[n] = value;
    // if the history buffer is full...
    n++;
    if (n >= history.size()) {
      // start a new history
      n = 0;

      // *maybe* copy the history buffer for the visual thread to use
      if (m.try_lock()) {
        memcpy(&history[0], &_history[0], history.size() * sizeof(float));
        m.unlock();
      }
    }
  }

  void operator()() {
    // get the lock (mutex); this will block, waiting for the audio thread to
    // release the lock. lock() waits while try_lock() does not.
    m.lock();

    // use the history buffer to draw the waveform
    ImGui::PlotLines("Waveform", &history[0], history.size(), 0, "", FLT_MAX,
                     FLT_MAX, ImVec2(0, 50));

    // make a copy
    float copy[history.size()];
    memcpy(copy, &history[0], history.size() * sizeof(float));
    // window the data
    for (unsigned i = 0; i < history.size(); ++i) copy[i] *= hann_window[i];
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
  }
};

}  // namespace ap

#endif
