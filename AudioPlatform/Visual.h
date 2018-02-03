#ifndef __240C_VISUAL__
#define __240C_VISUAL__

#include "GL/gl3w.h"
// order matters; must be first
#include <GLFW/glfw3.h>
//#include <stdio.h>
#include "imgui.h"                // external/imgui/
#include "imgui_impl_glfw_gl3.h"  // external/imgui/examples/opengl3_example/

namespace ap {

static void error_callback(int error, const char* description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

struct Visual {
  GLFWwindow* window = nullptr;
  Visual() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) exit(1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);
  }

  ~Visual() {
    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
  }

  virtual void visual() {}

  void loop() {
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
      ImGui_ImplGlfwGL3_NewFrame();

      visual();

      ImGui::Render();
      glfwSwapBuffers(window);
    }
  }
};

}  // namespace ap

#endif
