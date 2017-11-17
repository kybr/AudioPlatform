// Karl Yerkes
// karl.yerkes@gmail.com
// 2017-11-03
// MAT 240C
//

// this includes all the other headers; go take a look...
#include "Headers.h"

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
  Sine sineLeft, sineRight;
  Noise noise;

  Biquad biquadLeft, biquadRight;
  Line biquadLeftLine, biquadRightLine;

  App() {
    adsr.loop = true;
    left.other = &right;
    left.frequency(mtof(midi + tuneLine.value));
    right.frequency(mtof(midi + tuneLine.value));

    sineLeft.frequency(440.0f);
    sineRight.frequency(440.0f);
    noise.frequency(440.0f);
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      float tune = tuneLine.nextValue();
      float offset = offsetLine.nextValue();
      left.frequency(mtof(midi + tune));
      right.frequency(mtof(midi + tune + offset));
      biquadLeft.lpf(biquadLeftLine.nextValue(), 0.7f);
      biquadRight.lpf(biquadRightLine.nextValue(), 0.7f);
      sineLeft.increment = left.increment;
      sineRight.increment = right.increment;
      //
      float n = noise();
      float gain = gainLine.nextValue();
      float envelope = dbtoa(90.0 * (adsr.nextValue() - 1.0f));
      out[i + 0] = biquadLeft(left.nextValue() + sineLeft()) * gain * envelope;
      out[i + 1] =
          biquadRight(right.nextValue() + sineRight()) * gain * envelope;
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

      static float leftF0 = 127.0f, rightF0 = 127.0f;
      ImGui::SliderFloat("L Filter", &leftF0, 0.0f, 127.0f);
      ImGui::SliderFloat("R Filter ", &rightF0, 0.0f, 127.0f);
      biquadLeftLine.set(mtof(leftF0));
      biquadRightLine.set(mtof(rightF0));

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
