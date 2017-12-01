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
  bool applyADSR = false;
  int chooseRight = 0;

  Line rate;

  Line gainLine, tuneLine, offsetLine;

  ADSR adsr;
  Sine sineLeft, sineRight;
  Noise noise;

  Biquad biquadLeft;
  BiquadWithLines biquadRight;
  Line biquadLeftLine;
  float f0right = 10000.0f;

  SamplePlayer samplePlayer = SamplePlayer("/tmp/TingTing.wav");

  App() {
    adsr.loop = false;
    left.other = &right;
    left.frequency(mtof(midi + tuneLine.value));
    right.frequency(mtof(midi + tuneLine.value));

    sineLeft.frequency(440.0f);
    sineRight.frequency(440.0f);
    noise.frequency(440.0f);
  }

  void audio(float* out) {
    biquadRight.lpf(f0right, 1.7f);
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      float tune = tuneLine();
      float offset = offsetLine();
      left.frequency(mtof(midi + tune));
      right.frequency(mtof(midi + tune + offset));
      biquadLeft.lpf(biquadLeftLine(), 1.7f);

      samplePlayer.frequency(rate());

      //
      sineLeft.increment = left.increment;
      sineRight.increment = right.increment;

      //
      float n = noise();
      float gain = gainLine();

      float envelope = applyADSR ? dbtoa(90.0 * (adsr() - 1.0f)) : 1.0f;

      float lf = left();
      out[i + 0] = biquadLeft(lf) * gain * envelope;

      float rf = 0;
      switch (chooseRight) {
        default:
        case 0:
          rf += right();
          break;
        case 1:
          rf += sineRight();
          break;
        case 2:
          rf += samplePlayer();
          break;
      }

      out[i + 1] = biquadRight(rf) * gain * envelope;
    }

    static unsigned count = 0;
    if (count++ % 1000 == 0) printf("%f\n", out[0]);

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
      f0right = mtof(rightF0);

      ImGui::PlotLines("Scope left", &b[0], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));
      ImGui::PlotLines("Scope right", &b[1], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));

      //
      ImGui::Checkbox("Hard Sync", &left.sync);
      ImGui::Checkbox("ADSR", &applyADSR);
      ImGui::SliderInt("Choose Right", &chooseRight, 0, 2);

      static float bar = 1.0f;
      ImGui::SliderFloat("Sampler Player rate", &bar, -2.0f, 2.0f);
      rate.set(bar);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      {
        ImVec4 prevAlpha = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.0f;

        ImGui::Begin(
            "foo", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);

        // ImGui::Text("This!");
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 a(0.0f, 0.0f), b(100.0f, 100.0f);
        drawList->AddLine(a, b, 0xFF0000FF);
        ImGui::End();

        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = prevAlpha;
      }
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
