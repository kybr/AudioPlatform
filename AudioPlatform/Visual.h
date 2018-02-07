#ifndef __240C_VISUAL__
#define __240C_VISUAL__

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"

namespace ap {

static void error_callback(int error, const char* description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

struct Visual {
  GLFWwindow* window = nullptr;
  Visual() {
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) exit(1);
    window = glfwCreateWindow(1280, 720, "ImGui OpenGL2 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    ImGui_ImplGlfwGL2_Init(window, true);
  }

  ~Visual() {
    // Cleanup
    ImGui_ImplGlfwGL2_Shutdown();
    glfwTerminate();
  }

  virtual void visual() {}
  virtual void setup() {}

  void loop() {
    bool firstTime = true;
    if (firstTime) {
      firstTime = false;
      setup();
    }
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
  }
};

}  // namespace ap

#endif
