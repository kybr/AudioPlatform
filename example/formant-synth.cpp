#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

void make_hann(std::vector<float>& window) {
  for (unsigned n = 0; n < window.size(); ++n)
    window[n] = (1 - cos(2 * M_PI * n / (window.size() - 1))) / 2;
}

struct App : AudioVisual {
  const unsigned historySize = 4 * blockSize;
  FFT fft;

  Saw saw;
  Line gain;
  Line frequency;

  BiquadWithLines f1, f2, f3;

  float data[28][3]{
      {294, 2343, 3251}, {283, 2170, 2417}, {293, 2186, 2507},
      {333, 1482, 2232}, {329, 1806, 2723}, {295, 750, 2342},
      {360, 2187, 2830}, {401, 1833, 2241}, {334, 910, 2300},
      {434, 2148, 2763}, {462, 1659, 2127}, {415, 1955, 2421},
      {519, 1593, 2187}, {605, 1657, 2596}, {406, 727, 2090},
      {581, 1840, 2429}, {546, 1604, 2032}, {557, 1696, 2423},
      {581, 1439, 2186}, {707, 1354, 2289}, {541, 830, 2221},
      {766, 1782, 2398}, {688, 1446, 2314}, {806, 1632, 2684},
      {572, 1537, 1802}, {784, 1211, 2702}, {781, 1065, 2158},
      {652, 843, 2011},
  };

  std::mutex m;
  std::vector<float> history, _history;
  std::vector<float> hann;

  void setup() {
    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    hann.resize(historySize);
    make_hann(hann);
    fft.setup(historySize);
  }

  void audio(float* out) {
    static unsigned n = 0;

    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      saw.frequency(frequency());
      float s = saw();
      float f = f1(s) + f2(s) + f3(s) + 2 * s;
      f /= 5;
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
      static float db = -60.0f;
      ImGui::SliderFloat("Level (dB)", &db, -60.0f, 3.0f);
      gain.set(dbtoa(db), 50.0f);

      // make a slider for note value (frequency)
      static float note = 60;
      ImGui::SliderFloat("Frequency (MIDI)", &note, 0, 127);
      frequency.set(mtof(note), 50.0f);

      static float reson = 3;
      ImGui::SliderFloat("Resonance", &reson, 0.0001, 12);

      //
      static int form = 0;
      ImGui::SliderInt("Formant", &form, 0, 27);
      f1.bpf(data[form][0], reson);
      f2.bpf(data[form][1], reson);
      f3.bpf(data[form][2], reson);
      //

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

      /*
{
  ImGui::IsItemHovered();
}
*/

      ImGui::End();
    }

    {
      // ImGui::SetNextWindowPos(ImVec2(600, 20), ImGuiCond_FirstUseEver);
      // ImGui::ShowTestWindow();
    }
  }
};

int main() { App().start(); }
