#include <mutex>
#include "Headers.h"

// the AudioPlatform framework now uses the namespace "ap"
using namespace ap;

struct App : Visual, Audio {
  const unsigned historySize = 4 * blockSize;
  FFT fft;

  Sine sine;
  Line gain;
  Line frequency;

  std::mutex m;
  std::vector<float> history, _history;

  App() {
    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    fft.setup(historySize);
  }

  void audio(float* out) {
    // "static" variables are scoped to the block (in this case the function)
    // and their value is persistent. statics are also used in the GUI blocks
    // later.
    static unsigned n = 0;

    // for each pair of samples, left and right
    //
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      // set the frequency of the sine oscillator using the Line class to smooth
      // out jumps control values
      sine.frequency(frequency());

      // compute the next sample
      float f = sine() * gain();

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

  void visual() {
    {
      // make a slider for "volume" level
      static float db = -60.0f;
      ImGui::SliderFloat("Level (dB)", &db, -60.0f, 3.0f);
      gain.set(dbtoa(db), 50.0f);

      // make a slider for note value (frequency)
      static float note = 60;
      ImGui::SliderFloat("Frequency (MIDI)", &note, 0, 127);
      frequency.set(mtof(note), 50.0f);

      // get the lock (mutex); this will block, waiting for the audio thread to
      // release the lock. lock() waits while try_lock() does not.
      m.lock();

      // use the history buffer to draw the waveform
      ImGui::PlotLines("Waveform", &history[0], historySize, 0, "", FLT_MAX,
                       FLT_MAX, ImVec2(0, 50));

      // take the FFT
      fft.forward(&history[0]);

      // convert to dB scale on the y axis
      for (auto& f : fft.magnitude) f = atodb(f);

      // draw the spectrum
      ImGui::PlotLines("Spectrum", &fft.magnitude[0], fft.magnitude.size(), 0,
                       "", FLT_MAX, FLT_MAX, ImVec2(0, 50));

      // release the lock; this is important. if you don't release the lock,
      // then the audio thread can never get the lock, so it can never copy any
      // history and we'll stop getting updates
      m.unlock();
    }

    // You can ignore the stuff below this line ------------------------
    //
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.11, 0.11, 0.11, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // If you want to draw stuff using OpenGL, you would do that right here.
  }
};

int main() {
  App app;
  app.loop();
}
