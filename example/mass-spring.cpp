#include <functional>
#include <glm/vec2.hpp>
#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

// 4th order Runge-Kutta
// https://en.wikipedia.org/wiki/List_of_Runge%E2%80%93Kutta_methods#Classic_fourth-order_method
// https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
template <typename Float, typename State>
State rk4(const Float t0, const Float dt, const State u0,
          std::function<State(Float, State)> f) {
  State f0 = f(t0, u0);

  Float t1 = t0 + dt / 2.0;
  State u1 = u0 + dt * f0 / 2.0;
  State f1 = f(t1, u1);

  Float t2 = t0 + dt / 2.0;
  State u2 = u0 + dt * f1 / 2.0;
  State f2 = f(t2, u2);

  Float t3 = t0 + dt;
  State u3 = u0 + dt * f2;
  State f3 = f(t3, u3);

  return u0 + dt * (f0 + f1 * 2.0 + f2 * 2.0 + f3) / 6.0;
}

// Euler's method
// https://en.wikipedia.org/wiki/Euler_method
//
template <typename Float, typename State>
State euler(const Float t0, const Float dt, const State u0,
            std::function<State(Float, State)> f) {
  // y_n1 = y_n0 + h * f(t, y)
  return u0 + dt * f(t0, u0);
}

struct MassSpring {
  glm::dvec2 state{0};
  double springFactor{0}, dampingFactor{0};
  double f = 0, r = 0;

  // F = ma // Newton's Second Law
  // F = -kx // Hook's law
  // F = -cv // damping

  void reset(double velocity, double position = 0) {
    state = {position, velocity};
  }
  void recalculate() {
    double _f = f;
    _f /= (sampleRate / 2);
    _f *= 2 * M_PI;
    //    _f /= 2;  // this factor of 2 is mysterious :(

    // the natural frequency of *damped* harmonic oscillators is
    // not as straightforward as simple harmonic oscillators;
    // you have to do some math...
    double w0 = _f / sqrt(1 - r * r);
    springFactor = w0 * w0;
    dampingFactor = r * 2 * w0;

    // std::cout << "springFactor:" << springFactor << std::endl;
    // std::cout << "dampingFactor:" << dampingFactor << std::endl;
    // std::cout << "f:" << f << std::endl;
    // std::cout << "r:" << r << std::endl;
    // std::cout << "w0:" << w0 << std::endl;
  }

  void set(double f_, double r_) {
    f = f_;
    r = r_;
    recalculate();
  }

  void frequency(double f_) {
    f = f_;
    recalculate();
  }

  enum Method { DEFAULT = 0, EULER = 1, RK4 = 2, VERLET = 3 } method{DEFAULT};
  // make the next sample
  //
  double operator()() {
    switch (method) {
      default: {
        state.x += state.y;
        state.y += -springFactor * state.x + -dampingFactor * state.y;
      }

      case EULER: {
        auto dSdt = [=](double t, glm::dvec2 s) -> glm::dvec2 {
          return {s.y, -springFactor * s.x + -dampingFactor * s.y};
        };
        const int N = 100;
        const double dt = 1.0 / N;
        for (int i = 0; i < N; i++)
          state = euler<double, glm::dvec2>(0, dt, state, dSdt);
      } break;

      case RK4: {
        auto dSdt = [=](double t, glm::dvec2 s) -> glm::dvec2 {
          return {s.y, -springFactor * s.x + -dampingFactor * s.y};
        };
        state = rk4<double, glm::dvec2>(0, 1, state, dSdt);
      } break;

        // http://www.lonesock.net/article/verlet.html
        // https://www.saylor.org/site/wp-content/uploads/2011/06/MA221-6.1.pdf
        // https://en.wikipedia.org/wiki/Verlet_integration
        // http://codeflow.org/entries/2010/aug/28/integration-by-example-euler-vs-verlet-vs-runge-kutta/
      case VERLET: {
        const int N = 500;
        const double dt = 1.0 / N;
        double lastX = state.x;
        for (int i = 0; i < N; i++) {
          double v = (state.x - lastX);
          state.x = state.x + v +
                    (-springFactor * state.x + -dampingFactor * v) * dt * dt;
          lastX = state.x;
        }

      } break;
    }
    return state.x;
  }
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

      static int method = 0;
      ImGui::SliderInt("Method", &method, 0, 3);
      massSpring.method = (MassSpring::Method)method;

      static float dm = 60.0f;
      ImGui::SliderFloat("Damping (?)", &dm, 0, 1);
      damping.set(1 / mtof(dm * 135));

      // ImGui::SliderInt("passes (N)", &massSpring.stepsPerSample, 0, 15);

      ImGui::SliderFloat("Velocity (?)", &velocity, 0, 0.2);

      static float foo = 250;
      ImGui::SliderFloat("Rate (?)", &foo, 1, 500);
      t.ms(foo);

      // printf("%f %f %f\n", frequency.value, damping.value, velocity);

      soundDisplay();

      ImGui::End();
    }
  }
};

int main() { App().start(); }
