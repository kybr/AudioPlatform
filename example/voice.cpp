

#include <cmath>
#include <iostream>
#include <vector>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;
using namespace std;

// XXX do floats work?
//
#define double float

// TODO
//
// - gather vowel data and interpolate
// - map synthesis parameters to interface
// - complete loss model
// - investigate time-varying model
//

double vowel[] = {
    1.000000000000000, 0.696969696969697, 0.490909090909092, 2.400000000000000,
    2.400000000000000, 2.400000000000000, 3.200000000000000, 4.139393939393939,
    4.200000000000000, 4.200000000000000, 3.457575757575757, 2.850000000000000,
    2.319696969696970, 1.796969696969697, 1.645454545454546, 1.600000000000000,
    1.254545454545454, 0.800000000000000, 0.800000000000000, 1.672727272727273,
    2.000000000000000, 3.200000000000000, 3.200000000000000,
};

template <int N = 23>
struct Tube {
  // model parameters
  //
  double spaceStep, timeStep;
  double waveSpeed, tractLength, tractSurfaceAreaLeftEnd;
  double gamma, lambda, s0, r1, r2, g1;

  // state
  //
  double *p0, *p1, *p2, *s;

  Tube() {
    p0 = new double[N];
    p1 = new double[N];
    p2 = new double[N];
    s = new double[N];

    timeStep = 1.0 / 44100;                 // seonds
    spaceStep = 1.0 / (N - 1);              // no units
    waveSpeed = 340.0;                      // meter/second
    tractLength = 0.17;                     // meters
    tractSurfaceAreaLeftEnd = 0.00025;      // square meters
    gamma = waveSpeed / tractLength;        // Hertz :: 1/seconds
    lambda = gamma * timeStep / spaceStep;  // no units

    reset();
    recalculate();
  }

  void reset() {
    for (int i = 0; i < N; ++i) {
      p0[i] = p1[i] = p2[i] = 0.0;
      s[i] = vowel[i];
    }
  }

  void recalculate() {
    // TODO :: if possible, make each variable name meaningful
    double alf =
        2.0881 * tractLength * sqrt(1.0 / (tractSurfaceAreaLeftEnd * s[N - 1]));
    double bet = 0.7407 / gamma;
    double Sr = 1.5 * s[N - 1] - 0.5 * s[N - 2];
    double q1 =
        alf * gamma * gamma * timeStep * timeStep * Sr / (s[N - 1] * spaceStep);
    double q2 = bet * gamma * gamma * timeStep * Sr / (s[N - 1] * spaceStep);
    s0 = 2.0 * (1.0 - lambda * lambda);
    r1 = 2.0 * lambda * lambda / (1.0 + q1 + q2);
    r2 = -(1.0 + q1 - q2) / (1.0 + q1 + q2);
    g1 = -(timeStep * timeStep * gamma * gamma / spaceStep / s[0]) *
         (3.0 * s[0] - s[1]);
  }

  double operator()(double excitation) {
    // calculate the body of the tube
    //
    for (int k = 1; k < N - 1; ++k) {
      double mean = 0.25 * (s[k + 1] + s[k] + s[k] + s[k - 1]);
      p0[k] = 2.0 * (1.0 - lambda * lambda) * p1[k] +
              (0.5 * lambda * lambda / mean) * ((s[k] + s[k - 1]) * p1[k - 1] +
                                                (s[k] + s[k + 1]) * p1[k + 1]) -
              p2[k];
    }

    // calculate the right (radiation) end of the tube
    //
    p0[N - 1] = r1 * p1[N - 2] + r2 * p2[N - 1];

    // calculate the left (excitation) end of the tube
    //
    p0[0] = 2.0 * (1.0 - lambda * lambda) * p1[0] +
            2.0 * lambda * lambda * p1[1] - p2[0] + g1 * excitation;

    // sample the right end of the tube
    //
    double output = (p0[N - 1] - p1[N - 1]) / timeStep;

    double* temp = p2;
    p2 = p1;
    p1 = p0;
    p0 = temp;

    return output;
  }
};

struct MyApp : AudioVisual {
  SoundDisplay soundDisplay;
  Line gain;
  Line frequency;
  double width[23];

  Tube<> tube;
  Sine osc;

  void setup() { soundDisplay.setup(blockSize * 4); }

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

      for (int i = 0; i < 23; i++) {
        if (i > 0) ImGui::SameLine();
        ImGui::PushID(i);
        ImGui::VSliderFloat("##v", ImVec2(18, 160), &tube.s[i], 0.2, 5.0f, "");
        ImGui::PopID();
      }
      if (ImGui::Button("Button")) tube.reset();

      soundDisplay();

      ImGui::End();
    }

    // show test gui
    ImGui::ShowTestWindow();
  }

  void audio(float* out) {
    tube.recalculate();

    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      osc.frequency(frequency());
      double d = osc();
      double f = tube((d < 0.0) ? 0.0 : d) * gain();
      out[i + 1] = out[i + 0] = f;
      soundDisplay(f);
    }
  }
};

int main() { MyApp().start(); }
