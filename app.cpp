// Karl Yerkes
// karl.yerkes@gmail.com
// 2017-11-03
// MAT 240C
//

// this includes all the other headers; go take a look...
#include "Headers.h"

struct Line {
  float target, value, milliseconds, increment;

  Line() { set(0.0f, 0.0f, 30.0f); }

  void set() {
    increment = (target - value) / (sampleRate * (milliseconds / 1000.0f));
  }
  void set(float value, float target, float milliseconds) {
    this->value = value;
    this->target = target;
    this->milliseconds = milliseconds;
    set();  // sets increment based on the above
  }
  void set(float target, float milliseconds) {
    set(value, target, milliseconds);
  }
  void set(float target) { set(value, target, milliseconds); }

  bool done() {
    if (value == target) return true;
    // if the increment is negative, then we're done when the value is lower
    // than the target. alternatively, if the increment is positive, then
    // we're done when the value is greater than the target. in both cases, we
    // detect overshoot.
    return (increment < 0) ? (value < target) : (value > target);
  }

  float nextValue() {
    float returnValue = value;
    if (done())
      value = target;
    else
      value += increment;
    return returnValue;
  }
};

struct ADSR {
  float a, d, s, r;
  Line attack, decay, release;
  int state;
  bool loop = false;

  ADSR() { set(300.0f, 200.0f, 0.7f, 500.0f); }

  void reset() {
    state = 0;
    attack.set(0.0f, 1.0f, a);  // value, target, milliseconds
    decay.set(1.0f, s, d);
    release.set(s, 0.0f, r);
  }

  void set(float a, float d, float s, float r) {
    this->a = a;
    this->d = d;
    this->s = s;
    this->r = r;
    reset();
  }

  float nextValue() {
    switch (state) {
      case 0:
        if (attack.done()) state = 1;
        break;
      case 1:
        if (decay.done()) state = 2;
        break;
      case 2:
        if (release.done()) state = 3;
        break;
      default:
        if (loop) reset();
        break;
    }
    switch (state) {
      case 0:
        // cout << "a:";
        return attack.nextValue();
      case 1:
        // cout << "d:";
        return decay.nextValue();
      case 2:
        // cout << "r:";
        return release.nextValue();
      default:
        // we are spent; call trigger to start again
        // cout << "e:";
        return 0.0f;
    }
  }
};

//
struct App : Visual, Audio {
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  bool show_test_window = true;
  bool show_another_window = false;
  int midi = 60;
  float* b = new float[blockSize];
  HardSyncMultiSynth left, right;

  Line gainLine, tuneLine, offsetLine;

  ADSR adsr;

  App() {
    adsr.loop = true;
    left.other = &right;
    left.frequency(mtof(midi + tuneLine.value));
    right.frequency(mtof(midi + tuneLine.value));
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      float tune = tuneLine.nextValue();
      float offset = offsetLine.nextValue();
      left.frequency(mtof(midi + tune));
      right.frequency(mtof(midi + tune + offset));
      //
      float gain = gainLine.nextValue();
      float envelope = dbtoa(90.0 * (adsr.nextValue() - 1.0f));
      out[i + 0] = left.nextValue() * gain * envelope;
      out[i + 1] = right.nextValue() * gain * envelope;
    }
    memcpy(b, out, blockSize * channelCount * sizeof(float));
  }

  void visual() {
    {
      int was = midi;
      ImGui::SliderInt("Keyboard", &midi, 48, 72);
      if (midi != was) adsr.reset();

      static float tune = 0.0f;
      ImGui::SliderFloat("Tune", &tune, -12.0f, 12.0f);
      tuneLine.set(tune, 50.0f);

      static float offset = 0.0f;
      ImGui::SliderFloat("Offset", &offset, 0.0f, 12.0f);
      offsetLine.set(offset, 50.0f);

      static float db = -90.0f;
      ImGui::SliderFloat("dbtoa", &db, -90.0f, 9.0f);
      gainLine.set(dbtoa(db), 50.0f);

      ImGui::SliderInt("Osc left", &left.type, 0, 3);
      ImGui::SliderInt("Osc right", &right.type, 0, 3);

      ImGui::PlotLines("Scope left", &b[0], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));
      ImGui::PlotLines("Scope right", &b[1], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));

      ImGui::Checkbox("Hard Sync", &left.sync);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    {
      ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
      ImGui::ShowTestWindow(&show_test_window);
    }

    // You can ignore the stuff below this line ------------------------
    //
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // If you want to draw stuff using OpenGL, you would do that right here.
  }
};

int main() {
  App app;
  app.loop();
}
