# AudioPlatform

is a mini-framework for teaching digital audio programming


## Dependenceies
- GLFW
- ImGui
- rtaudio.
- rtmidi


## Windows

Execute the `build.bat` script from a Visual Studio Developer's Terminal. If this does not work, try rebuilding rtaudio and/or ImGui; then try again. The build process creates a file `Debug\app.exe` which you may execute by typing that on the Terminal.


## macOS

First, ensure that the _glfw_ library is installed with `brew install glfw`. On the terminal, use the `make` command to build this framework. Once built, run by saying `./app` on the terminal.


## File Manifest

- `README.md` is this file
+ mini-framework "wrapper" classes
  - `Audio.h` wraps RtAudio, presenting a much-simplified user-facing API
  - `Visual.h`  wraps ImGui and GLFW, offering a visual callback to the user
- `Makefile` contains a information that `make` uses to build this mini-framework
- `app` is the executable binary that gets built; execute with `./app` on the terminal
- **`app.cpp` is the only file you will edit**
- `rtaudio` is a folder containing the source code of the [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/) library
+ Windows specific
  - `build.bat` builds the mini-framework on Windows
  - `Debug` is the folder where the `app.exe` gets put
+ ImGui stuff
  - `imgui` is a folder containing the source code of the [ImGui](https://github.com/ocornut/imgui/) library
  - `imgui.ini`
  - `imgui_impl_glfw_gl3.cpp`
  - `imgui_impl_glfw_gl3.h`
+ `*.o` are "object files"; they contain machine-readable functions and data
