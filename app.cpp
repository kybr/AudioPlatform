#include "Headers.h"

struct App : Visual, Audio, MIDI {
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  bool show_test_window = true;
  bool show_another_window = false;
  // this is an array / pointer to memory
  float* waveformData = new float[blockSize * channelCount];

  Sine sine;
  Line gain;
  Line envelope;
  Timer t;

  Table oscillator;
  Biquad filter;

  float pulseRate = 200;

  App() {
    sine.frequency(440.0f);

    // Table setup
    // size == 4096
    for (int i = 0; i < oscillator.size; i++)
      oscillator.data[i] = sin(M_PI * i / oscillator.size);
    oscillator.frequency(220);

    // Biquad setup
    filter.bpf(700, 2.5);
  }

  void audio(float* out) {
    // for each pair of samples, left and right
    //
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      if (t()) {
        envelope.set(1, 0, pulseRate);
      }
      float e = envelope();
      float g = gain();
      // float s = filter(oscillator());  // sine();
      // float s = oscillator() + filter(oscillator());  // sine();
      float o = oscillator();
      float s = o + filter(o);  // sine();
      out[i + 1] = s * g * e;
      out[i + 0] = s * g * e;
    }

    // copy
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

      static float note = 45;
      ImGui::SliderFloat("note", &note, 0, 127);
      oscillator.frequency(mtof(note));

      float was = pulseRate;
      ImGui::SliderFloat("pulse rate", &pulseRate, 0, 1000);
      if (was != pulseRate) t.period(pulseRate / 1000);

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
