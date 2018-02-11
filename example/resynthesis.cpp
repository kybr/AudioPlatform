#include <fstream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Synths.h"

using namespace std;
using namespace ap;

template <typename Out>
void split(const string& s, char delim, Out result) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    *(result++) = item;
  }
}

vector<string> split(const string& s, char delim) {
  vector<string> elems;
  split(s, delim, back_inserter(elems));
  return elems;
}

struct Data {
  float frequency, magnitude;
};
vector<vector<Data>> data;

vector<Array> frequency, magnitude;

void load(string dataFile, vector<vector<Data>>& data) {
  ifstream in(dataFile);
  if (!in.good()) {
    printf("ERROR: failed to load file: %s\n", dataFile.c_str());
    exit(10);
  }
  string line;
  while (getline(in, line)) {
    vector<string> dataPoint = split(line, ' ');
    data.push_back(vector<Data>());
    for (auto& s : dataPoint) {
      vector<string> pair = split(s, ':');
      data.back().push_back({stof(pair[0]), stof(pair[1])});
    }
  }

  frequency.resize(data[0].size());
  for (auto& a : frequency) a.resize(data.size());
  for (unsigned i = 0; i < frequency.size(); ++i)
    for (unsigned j = 0; j < frequency[0].size; ++j)
      frequency[i][j] = data[j][i].frequency;
  magnitude.resize(data[0].size());
  for (auto& a : magnitude) a.resize(data.size());
  for (unsigned i = 0; i < frequency.size(); ++i)
    for (unsigned j = 0; j < frequency[0].size; ++j)
      magnitude[i][j] = data[j][i].magnitude;
}

struct App : AudioVisual {
  const unsigned historySize = 4 * blockSize;
  FFT fft;

  vector<Sine> sine;
  vector<Line> gain;
  vector<Line> freq;
  Line masterGain;
  Line position;

  mutex m;
  vector<float> history, _history;

  void setup() {
    history.resize(historySize, 0);
    _history.resize(historySize, 0);
    fft.setup(historySize);

    sine.resize(data[0].size());
    gain.resize(data[0].size());
    freq.resize(data[0].size());

    //    for (unsigned i = 0; i < data[0].size(); ++i) {
    //      freq[i].milliseconds = 15;
    //      gain[i].milliseconds = 15;
    //    }
  }

  void audio(float* out) {
    static unsigned n = 0;

    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      for (unsigned i = 0; i < sine.size(); ++i) sine[i].frequency(freq[i]());
      float f = 0;
      for (unsigned i = 0; i < sine.size(); ++i) f += sine[i]() * gain[i]();
      f /= sine.size();
      f *= masterGain();
      out[i + 1] = out[i + 0] = f;

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
    // this stuff makes a single "root" window
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    ImGui::SetWindowPos("window", ImVec2(0, 0));
    ImGui::SetWindowSize("window", ImVec2(windowWidth, windowWidth));
    ImGui::Begin("window", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize);

    static float db = -60.0f;
    ImGui::SliderFloat("Level (dB)", &db, -60.0f, 3.0f);
    masterGain.set(dbtoa(db), 50.0f);

    static float increment = 0.35;
    ImGui::SliderFloat("Playback Rate", &increment, -1, 1);

    static float shift = 1;
    ImGui::SliderFloat("Frequency Shift", &shift, 0.4, 2.1);

    static float where = 0.5;
    where += increment * 0.009;
    if (where > 1) where -= 1;
    if (where < 0) where += 1;
    ImGui::SliderFloat("Postion", &where, 0, 1);

    float index = where * frequency[0].size;
    for (unsigned i = 0; i < frequency.size(); ++i) {
      freq[i].set(frequency[i].get(index) * shift);
      gain[i].set(magnitude[i].get(index));
    }
    /*
    unsigned index = floor(where * data.size());
    unsigned i = 0;
    for (auto& pair : data[index]) {
      freq[i].set(pair.frequency * shift);
      gain[i].set(pair.magnitude);
      i++;
    }
    */

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

int main(int argc, char* argv[]) {
  if (argc == 2) {
    load(argv[1], data);
    printf("loaded %s\n", argv[1]);
  } else
    load("media/TingTing.wav.txt", data);

  App().start();
}
