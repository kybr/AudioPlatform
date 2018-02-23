#include <fstream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
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
  SoundDisplay soundDisplay;

  vector<Sine> sine;
  vector<Line> gain;
  vector<Line> freq;
  Line masterGain;
  Line position;

  void setup() {
    soundDisplay.setup(4 * blockSize);

    sine.resize(data[0].size());
    gain.resize(data[0].size());
    freq.resize(data[0].size());

    for (unsigned i = 0; i < data[0].size(); ++i) {
      freq[i].milliseconds = 15;
      gain[i].milliseconds = 15;
    }
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      for (unsigned i = 0; i < sine.size(); ++i) sine[i].frequency(freq[i]());
      float f = 0;
      for (unsigned i = 0; i < sine.size(); ++i) f += sine[i]() * gain[i]();
      f /= sine.size();
      f *= masterGain();
      f *= 3;  // because its sorta quiet
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

    soundDisplay();

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
