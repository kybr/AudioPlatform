# AudioPlatform

This is a framework for learning/teaching digital audio programming.

![](Formant-Synth.png)

- - -

**Goals:**

- Few dependencies
- Conceptually simple implementations, when possible
- Permissive licenses
- Portable; It should build and run on:
  + Linux
  + macOS
  + (TBD) Windows
  + (TBD) The browser via [Emscripten](https://github.com/kripken/emscripten)

**Status:**

Currently GLWF3 is the only library dependency. RtAudio and others build along with user code.

- - -

## Getting Started

Media (e.g., .wav and .png files) are held using [Git Large File Storage](https://git-lfs.github.com), so you'll need to install `git-lfs` on your system.

Prepare your development environment:

- Linux

  On apt-based systems, you'll need `libglfw3-dev` and `libasound2-dev`. The `build-essential` package will install everything you need to compile and link.

- macOS

  First, install Xcode with `xcode-select --install` on the terminal. Then, install [Homebrew](https://brew.sh). Finally, install the _glfw_ library with `brew install glfw`.

- Windows (TDB)

  TDB: This will probably use MinGW, but maybe `choco`, `vcbuildtools`, and `vcpkg`.

### Building examples

To build and run an example, use the `run` script. For instance, `./run example/simple.cpp` will build and run the example `example/simple.cpp`

This works for any .cpp files in some subfolder of this repo, so if you make a folder `foo` and a file `foo/bar.cpp`, you should be able to build and run with `./run foo/bar.cpp`.
