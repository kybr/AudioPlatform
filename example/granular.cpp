#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/Functions.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

#include <algorithm>
#include <set>
#include <vector>

using namespace std;
using namespace ap;

// http://bingweb.binghamton.edu/~hhu1/pitch/YAPT.pdf
// https://pdfs.semanticscholar.org/4272/e26917c4a9332cf5991bbc476d3ebdcb5142.pdf
//

Array hann_window;
FFT fft;
vector<float> fft_input;

struct Grain : Array {
  float rms, zcr, centroid, pitch;

  Grain(const Array& clip, unsigned begin, unsigned end) {
    // zero crossing rate

    zcr = 0;
    for (unsigned i = 1 + begin; i < end; ++i)
      if (clip[i] * clip[i - 1] < 0) zcr++;
    zcr = sampleRate * zcr / (end - begin) / 2;

    // copy and window the grain
    resize(end - begin);
    for (unsigned i = 0; i < size; ++i) {
      float windowIndex = hann_window.size * float(i) / size;
      data[i] = clip[begin + i] * hann_window.get(windowIndex);
    }

    // find the rms
    rms = 0;
    for (unsigned i = 0; i < size; ++i) rms += data[i] * data[i];
    rms /= size;
    rms = sqrt(rms);

    // fft with zero padding
    memset(&fft_input[0], 0, sizeof(float) * fft_input.size());
    memcpy(&fft_input[0], &data[0], sizeof(float) * size);
    fft.forward(&fft_input[0]);

    // spectral centroid
    centroid = 0;
    for (unsigned i = 0; i < fft.magnitude.size(); ++i)
      centroid += fft.magnitude[i] * i / fft.magnitude.size() * sampleRate / 2;
    float denominator = 0;
    for (unsigned i = 0; i < fft.magnitude.size(); ++i)
      denominator += fft.magnitude[i];
    centroid /= denominator;

    return;

    // autocorrelation method

    struct Data {
      float value, frequency;
    };
    vector<Data> d;
    for (int lag = 0; lag < (int)size; lag++) {
      float r = 0;
      for (int n = 0; n < (int)size; n++) {
        if ((n - lag) < 0)
          ;
        else
          r += data[n] * data[n - lag];
      }
      d.push_back({abs(r), sampleRate / lag});
    }

    sort(d.begin(), d.end(),
         [](const Data& a, const Data& b) { return a.value > b.value; });

    pitch = d[1].frequency;

    printf("%.3f\t%.3f\t%.3f\t%.3f\n", zcr, rms, centroid, pitch);
  }

  unsigned index = 0;
  float operator()() { return nextValue(); }
  float nextValue() { return data[index]; }
  bool hasNext() {
    if (index < size) {
      index++;
      return true;
    }
    index = 0;
    return false;
  }
};

struct Cloud {
  vector<Grain*> grain;
  set<Grain*> playlist;
  vector<Grain*> rms, centroid, zcr;

  void setup(SamplePlayer& player, unsigned length, unsigned hop) {
    for (unsigned i = 0; i < player.size - length * 2; i += hop) {
      grain.push_back(new Grain(player, i, i + length));
    }

    for (auto p : grain) {
      rms.push_back(p);
      centroid.push_back(p);
      zcr.push_back(p);
    }

    sort(rms.begin(), rms.end(),
         [](Grain* a, Grain* b) { return a->rms < b->rms; });
    sort(centroid.begin(), centroid.end(),
         [](Grain* a, Grain* b) { return a->centroid < b->centroid; });
    sort(zcr.begin(), zcr.end(),
         [](Grain* a, Grain* b) { return a->zcr < b->zcr; });
  }

  unsigned index = 0;
  void appendNext() {
    // playlist.insert(grain[index]);
    // playlist.insert(rms[index]);
    // playlist.insert(zcr[index]);
    playlist.insert(centroid[index]);
    index++;
    if (index >= grain.size()) index = 0;
  }

  float operator()() { return nextValue(); }
  float nextValue() {
    float sum = 0;
    set<Grain*> shouldRemove;
    for (auto g : playlist) {
      if (g->hasNext())
        sum += g->nextValue();
      else
        shouldRemove.insert(g);
    }
    for (auto g : shouldRemove) playlist.erase(g);
    return sum;
  }
};

struct App : AudioVisual {
  SoundDisplay soundDisplay;
  SamplePlayer player;
  Line gain;
  Line frequency;

  Cloud cloud;
  Timer timer;

  void setup() {
    player.load("media/Impulse-Sweep.wav");
    // player.load("media/Sine-Sweep.wav");
    // player.load("media/Saw-Sweep.wav");
    // player.load("media/TingTing.wav");
    soundDisplay.setup(4 * blockSize);

    hann(hann_window, 8192);
    fft_input.resize(16384);
    fft.setup(fft_input.size());

    timer.ms(200);

    unsigned length = sampleRate * 0.05;
    unsigned hop = length / 3;
    cloud.setup(player, length, hop);
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      timer.frequency(frequency());
      // float f = player();
      if (timer()) {
        cloud.appendNext();
      }
      float f = cloud();
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
      static float db = -60.0f;
      ImGui::SliderFloat("Level (dB)", &db, -60.0f, 3.0f);
      gain.set(dbtoa(db), 50.0f);

      // make a slider for note value (frequency)
      static float M = 0;
      ImGui::SliderFloat("Rate (MIDI)", &M, -5, 60);
      frequency.set(mtof(M), 20.0f);

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
