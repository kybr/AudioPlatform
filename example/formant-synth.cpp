#include <mutex>
#include "AudioPlatform/AudioVisual.h"
#include "AudioPlatform/FFT.h"
#include "AudioPlatform/SoundDisplay.h"
#include "AudioPlatform/Synths.h"

using namespace ap;

struct App : AudioVisual {
  SoundDisplay soundDisplay;
  Saw saw;
  Line gain;
  Line frequency;

  BiquadWithLines f1, f2, f3;

  float data[28][3]{
      {294, 2343, 3251}, {283, 2170, 2417}, {293, 2186, 2507},
      {333, 1482, 2232}, {329, 1806, 2723}, {295, 750, 2342},
      {360, 2187, 2830}, {401, 1833, 2241}, {334, 910, 2300},
      {434, 2148, 2763}, {462, 1659, 2127}, {415, 1955, 2421},
      {519, 1593, 2187}, {605, 1657, 2596}, {406, 727, 2090},
      {581, 1840, 2429}, {546, 1604, 2032}, {557, 1696, 2423},
      {581, 1439, 2186}, {707, 1354, 2289}, {541, 830, 2221},
      {766, 1782, 2398}, {688, 1446, 2314}, {806, 1632, 2684},
      {572, 1537, 1802}, {784, 1211, 2702}, {781, 1065, 2158},
      {652, 843, 2011},
  };

  void setup() { soundDisplay.setup(4 * blockSize); }

  void audio(float* out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      saw.frequency(frequency());
      float s = saw();
      float f = f1(s) + f2(s) + f3(s) + 2 * s;
      f /= 5;
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
      static float note = 60;
      ImGui::SliderFloat("Frequency (MIDI)", &note, 0, 127);
      frequency.set(mtof(note), 50.0f);

      static float reson = 3;
      ImGui::SliderFloat("Resonance", &reson, 0.0001, 12);

      //
      static int form = 0;
      ImGui::SliderInt("Formant", &form, 0, 27);
      f1.bpf(data[form][0], reson);
      f2.bpf(data[form][1], reson);
      f3.bpf(data[form][2], reson);
      //

      soundDisplay();

      ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x,
                  ImGui::GetIO().MousePos.y);
      ImGui::Text("Mouse State - 0:%d 1:%d 2:%d", ImGui::GetIO().MouseDown[0],
                  ImGui::GetIO().MouseDown[1], ImGui::GetIO().MouseDown[2]);

      ImGui::Text("Mouse Clicked - 0:%d 1:%d", ImGui::IsMouseClicked(0),
                  ImGui::IsMouseClicked(1));

      ImGui::End();
    }

    {
      // ImGui::SetNextWindowPos(ImVec2(600, 20), ImGuiCond_FirstUseEver);
      // ImGui::ShowTestWindow();
    }
  }
};

int main() { App().start(); }
