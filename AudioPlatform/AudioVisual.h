#ifndef __AP_AUDIO_VISUAL__
#define __AP_AUDIO_VISUAL__

// OpenGL
#include <GLFW/glfw3.h>

// AudioIO
#include "rtaudio/RtAudio.h"

// GUI
#include "imgui.h"
#include "imgui_impl_glfw.h"

#include <map>

namespace ap {

float mtof(float m);
float ftom(float f);
float dbtoa(float db);
float atodb(float a);

class AudioVisual {
 public:
  GLFWwindow *window = nullptr;
  RtAudio *dac = nullptr;

  virtual void setup() = 0;
  virtual void visual() = 0;
  virtual void audio(float *out) = 0;
  void start();
};

}  // namespace ap

#endif
