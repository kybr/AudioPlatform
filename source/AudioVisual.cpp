#include "AudioPlatform/AudioVisual.h"

// OpenGL
#include <GLFW/glfw3.h>

// AudioIO
#include "rtaudio/RtAudio.h"

// GUI
#include "imgui.h"
#include "imgui_impl_glfw.h"

#include <map>
#include <cmath>

#include "AudioPlatform/Globals.h"

namespace ap {

float mtof(float m) { return 8.175799 * powf(2.0f, m / 12.0f); }
float ftom(float f) { return 12.0f * log2f(f / 8.175799); }
float dbtoa(float db) { return 1.0f * powf(10.0f, db / 20.0f); }
float atodb(float a) { return 20.0f * log10f(a / 1.0f); }

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

static int chooseDevice(RtAudio &audio) {
  RtAudio::DeviceInfo info;

  unsigned int devices = audio.getDeviceCount();

  for (unsigned int i = 0; i < devices; i++) {
    info = audio.getDeviceInfo(i);

    std::size_t found = info.name.find("HDMI");
    if (found != std::string::npos) {
      continue;
    }

    if (info.probed == false) {
      printf("Device %d probe failed\n", i);
      continue;
    }
    if (info.inputChannels < 2) {
      continue;
    }
    if (info.outputChannels < 2) {
      continue;
    }

    printf("Audio Device: %s\n", info.name.c_str());
    return i;
  }
  return 0;
}

static int cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
              double streamTime, RtAudioStreamStatus status, void *data) {
  reinterpret_cast<AudioVisual *>(data)->audio((float *)outputBuffer);
  if (status) std::cout << "Stream underflow detected!" << std::endl;
  return 0;
}

void AudioVisual::start() {
  setup();
  std::map<int, std::string> apiMap;
  apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
  apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
  apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
  apiMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
  apiMap[RtAudio::UNIX_JACK] = "Jack Client";
  apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
  apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
  apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
  apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

  std::vector<RtAudio::Api> apis;
  RtAudio::getCompiledApi(apis);

  /*
  std::cout << "\nRtAudio Version " << RtAudio::getVersion() << std::endl;

  std::cout << "\nCompiled APIs:\n";
  for (unsigned int i = 0; i < apis.size(); i++)
    std::cout << "  " << apiMap[apis[i]] << std::endl;
*/
  ////////////////

  dac = new RtAudio();

  unsigned int deviceCount = dac->getDeviceCount();
  if (deviceCount == 0) {
    std::cout << "\nNo audio devices found!\n";
    exit(1);
  }

  dac->showWarnings(true);

  RtAudio::StreamParameters oParams;
  oParams.deviceId = 0;
  oParams.nChannels = channelCount;
  oParams.firstChannel = 0;
  oParams.deviceId = chooseDevice(*dac);

  RtAudio::StreamOptions options;
  // options.flags = RTAUDIO_HOG_DEVICE;
  options.flags |= RTAUDIO_SCHEDULE_REALTIME;

  try {
    unsigned bs = blockSize;
    dac->openStream(&oParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bs, &cb,
                    (void *)this, &options, nullptr);
    if (bs != blockSize) {
      printf("FAIL");
      exit(2);
    }
    dac->startStream();
  } catch (RtAudioError &e) {
    e.printMessage();
    if (dac->isStreamOpen()) dac->closeStream();
    exit(1);
  }

  // Setup window
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) exit(1);

  //    printf("GOT HERE\n");
  window = glfwCreateWindow(1280, 720, "This is the truth", nullptr, nullptr);
  if (window == nullptr) {
    printf("Failed to make a window\n");
    exit(-1);
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync
  ImGui_ImplGlfwGL2_Init(window, true);

  //
  //    int windowWidth, windowHeight;
  //    glfwGetWindowSize(window, &windowWidth, &windowHeight);
  //    printf("Window is (%d, %d)\n", windowWidth, windowHeight);
  //    fflush(stdout);

  while (!glfwWindowShouldClose(window)) {
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application. Generally you may always pass all inputs
    // to dear imgui, and hide them from your application based on those two
    // flags.
    glfwPollEvents();
    ImGui_ImplGlfwGL2_NewFrame();

    visual();

    ImGui::Render();
    glfwSwapBuffers(window);
  }

  try {
    dac->stopStream();
  } catch (RtAudioError &e) {
    e.printMessage();
  }
  ImGui_ImplGlfwGL2_Shutdown();
  glfwTerminate();
}

}  // namespace ap
