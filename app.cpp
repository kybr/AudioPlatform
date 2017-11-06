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
  float gain = 0.0f;
  float tune = 0.0f;
  int midi = 60;
  float db = -7.0f;
  float offset = 0.0f;
  float* b = new float[blockSize];
  HardSyncMultiSynth left, right;

  App() {
    left.other = &right;
    left.frequency(mtof(midi + tune));
    right.frequency(mtof(midi + tune));
  }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      out[i + 0] = left.nextValue() * gain;
      out[i + 1] = right.nextValue() * gain;
    }
    memcpy(b, out, blockSize * channelCount * sizeof(float));
  }

  void visual() {
    {
      ImGui::SliderFloat("Tune", &tune, -12.0f, 12.0f);
      ImGui::SliderFloat("Offset", &offset, 0.0f, 12.0f);
      ImGui::SliderInt("Keyboard", &midi, 48, 72);
      left.frequency(mtof(tune + midi));
      right.frequency(mtof(tune + midi + offset));

      ImGui::SliderFloat("dbtoa", &db, -90.0f, 9.0f);
      gain = dbtoa(db);

      ImGui::SliderInt("Osc left", &left.type, 0, 3);
      ImGui::SliderInt("Osc right", &right.type, 0, 3);

      ImGui::PlotLines("Scope left", &b[0], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));
      ImGui::PlotLines("Scope right", &b[1], blockSize, 0, nullptr, FLT_MAX,
                       FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));

      if (ImGui::Button("Hard Sync")) left.sync = !left.sync;

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
