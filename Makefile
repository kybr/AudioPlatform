.SUFFIXES:

EXE = $(MAKECMDGOALS)

CXX=
CXX += c++
CXX += -std=c++14
CXX += -g
CXX += -Wall
CXX += -Wextra
CXX += -Wno-unused-parameter
#CXX += -Werror
#CXX += -O3

INC=
INC += -I ./
INC += -I external/ 
INC += -I external/imgui/
INC += -I external/imgui/examples/opengl2_example/

LIB=

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX

	CXX += -D__LINUX_ALSA__
	LIB += -lpthread
	LIB += -lglfw
	LIB += -ldl
	LIB += -lasound
	LIB += -lGL
endif

ifeq ($(UNAME_S), Darwin) #APPLE

	CXX += -D__MACOSX_CORE__

	INC += -I/usr/local/include/

	LIB += -L/usr/local/lib/
	LIB += -lpthread
	LIB += -lglfw
	LIB += -framework CoreFoundation
	LIB += -framework CoreAudio
	LIB += -framework CoreMIDI
	LIB += -framework OpenGL
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
endif

OBJ=
OBJ += external/AudioFFT/AudioFFT.o
OBJ += external/imgui/examples/opengl2_example/imgui_impl_glfw.o
OBJ += external/imgui/imgui.o
OBJ += external/imgui/imgui_demo.o
OBJ += external/imgui/imgui_draw.o
OBJ += external/rtaudio/RtAudio.o
OBJ += external/rtmidi/RtMidi.o
OBJ += source/AudioVisual.o
OBJ += source/MIDI.o
OBJ += source/Functions.o
OBJ += source/Types.o
OBJ += source/Wav.o
OBJ += source/FFT.o

HDR=
HDR += AudioPlatform/AudioVisual.h
HDR += AudioPlatform/FFT.h
HDR += AudioPlatform/Globals.h
HDR += AudioPlatform/MIDI.h
HDR += AudioPlatform/Functions.h
HDR += AudioPlatform/Types.h
HDR += AudioPlatform/Synths.h
HDR += AudioPlatform/Wav.h

libap.a: $(OBJ) $(HDR)
	ar r $@ $^


$(EXE): $(EXE).cpp libap.a
	$(CXX) $(INC) -o $@.exe $^ $(LIB)

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $^

%.a: %.o

%.o: %.c
	cc $(INC) -c -o $@ $<
