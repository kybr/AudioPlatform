#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

// F = ma // Newton's Second Law
// F = -kx // Hook's law
// F = -cv // damping

struct MassSpring {
  float position = 0.0f, velocity = 0.03f;
  float springFactor, dampingFactor;
  int stepsPerSample = 1;

  void reset(float velocity_ = 0.03) { velocity = velocity_; }

  float f = 0, r = 0;
  void recalculate() {
    f /= (sampleRate / 2);
    f *= 2 * M_PI;
    f /= 2;  // this factor of 2 is mysterious :(

    // the natural frequency of *damped* harmonic oscillators is
    // not as straightforward as simple harmonic oscillators;
    // you have to do some math...
    float w0 = f / sqrt(1 - r * r);
    springFactor = w0 * w0;
    dampingFactor = r * 2 * w0;
  }
  void set(float f_, float r_) {
    f = f_;
    r = r_;
    recalculate();
  }

  void frequency(float f_) {
    f = f_;
    recalculate();
  }

  void damping(float r_) {
    r = r_;
    recalculate();
  }

  void step() {
    // Euler's method
    velocity +=
        (-springFactor * position + -dampingFactor * velocity) / stepsPerSample;
    position += velocity / stepsPerSample;
  }

  float nextValue() {
    for (unsigned i = 0; i < stepsPerSample; ++i) step();
    return position;
  }
  float operator()() { return nextValue(); }
};

struct App : AudioVisual {
  SoundDisplay soundDisplay;
  MassSpring massSpring;
  Line gain, frequency, damping;
  Timer t;
  float velocity = 0.03f;

  void setup() {
    t.ms(250);
    massSpring.frequency(440);
    soundDisplay.setup(4 * blockSize);
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      float f = 0;
      massSpring.set(frequency(), damping());
      if (t()) {
        massSpring.reset(velocity);
      }
      f = massSpring();
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

      static float m = 60.0f;
      ImGui::SliderFloat("Frequency (Hz)", &m, 0, 127);
      frequency.set(mtof(m));

      static float dm = 60.0f;
      ImGui::SliderFloat("Damping (?)", &dm, 0, 1);
      damping.set(1 / mtof(dm * 135));
      printf("%f\n", 1 / mtof(dm * 135));

      ImGui::SliderInt("passes (N)", &massSpring.stepsPerSample, 0, 15);

      ImGui::SliderFloat("Velocity (?)", &velocity, 0, 0.2);

      static float foo = 250;
      ImGui::SliderFloat("Rate (?)", &foo, 1, 500);
      t.ms(foo);

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
