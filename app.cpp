#include "Headers.h"

struct App : Visual, Audio, MIDI {
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  bool show_test_window = true;
  bool show_another_window = false;
  float* waveformData = new float[blockSize];

  Sine sine;
  Line gain;

  App() { sine.frequency(440.0f); }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      float g = gain();
      out[i + 1] = out[i + 0] = sine() * g;
    }
    memcpy(waveformData, out, blockSize * channelCount * sizeof(float));
  }

  std::vector<unsigned char> message;
  void visual() {
    // poll midi events
    midi(message);
    for (int i = 0; i < message.size(); ++i)
      std::cout << "midi byte: " << (int)message[i] << std::endl;

    // GUI stuff
    {
      static float db = -90.0f;
      ImGui::SliderFloat("dbtoa", &db, -90.0f, 9.0f);
      gain.set(dbtoa(db), 50.0f);

      // draw waveforms
      ImGui::PlotLines("Scope left", &waveformData[0], blockSize, 0, nullptr,
                       FLT_MAX, FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));
      ImGui::PlotLines("Scope right", &waveformData[1], blockSize, 0, nullptr,
                       FLT_MAX, FLT_MAX, ImVec2(0, 0), 2 * sizeof(float));

      // say frames per second
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      // draw a little red line
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

    // show the demo window
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
