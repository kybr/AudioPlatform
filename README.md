# AudioPlatform

This is a mini-framework for teaching digital audio programming.

Goals:

- Few dependencies
- Conceptually simple implementations
- Permissive licenses
- [Emscripten](https://github.com/kripken/emscripten)-able to browsers

![](Formant-Synth.png)


## Getting started


- Linux

  On apt-based systems, you'll need `libglfw3-dev` and `libasound2-dev`. The `build-essential` package will install everything you need to compile and link. 

- Windows

  Execute the `build.bat` script from a Visual Studio Developer's Terminal. If this does not work, try rebuilding rtaudio and/or ImGui; then try again. The build process creates a file `Debug\app.exe` which you may execute by typing that on the Terminal.

- macOS

  First, ensure that the _glfw_ library is installed with `brew install glfw`. On the terminal, use the `make` command to build this framework. Once built, run by saying `./app` on the terminal.
