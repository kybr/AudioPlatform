# Karl Yerkes
# karl.yerkes@gmail.com
# 2017-11-03
# MAT 240C
#
#
.SUFFIXES:

CXX=
CXX += c++
CXX += -std=c++11
CXX += -Wall
CXX += -Wformat
CXX += -g

INC=
# AudioPlatform
INC += -I ./
# rtaudio, rtmidi, AudioFFT, AudioFile
INC += -I external/ 
# imgui
INC += -I external/imgui/
INC += -I external/imgui/examples/opengl2_example/

LIB=

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"

	CXX += -D__LINUX_ALSA__

	LIB += -lpthread
	LIB += -lglfw
	LIB += -ldl
	LIB += -lasound
	LIB += -lGL
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"

	CXX += -D__MACOSX_CORE__

	INC += -I/usr/local/include/

	LIB += -L/usr/local/lib/
	LIB += -lpthread
	LIB += -lglfw
	LIB += -framework CoreFoundation
	LIB += -framework CoreAudio
	LIB += -framework CoreMIDI

# need these?
	LIB += -framework OpenGL
	LIB += -framework Cocoa
	LIB += -framework IOKit
	LIB += -framework CoreVideo
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
endif

OBJ=
OBJ += external/AudioFile/AudioFile.o
OBJ += external/rtaudio/RtAudio.o
OBJ += external/rtmidi/RtMidi.o
OBJ += external/imgui/examples/opengl2_example/imgui_impl_glfw.o
OBJ += external/imgui/imgui.o
OBJ += external/imgui/imgui_demo.o
OBJ += external/imgui/imgui_draw.o
OBJ += external/AudioFFT/AudioFFT.o

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $<

%.o: %.c
	cc $(INC) -c -o $@ $<

_: example/fm-synth example/formant-synth app

app: app.o $(OBJ)
	$(CXX) -o $@ $^ $(LIB) 

example/formant-synth: example/formant-synth.o $(OBJ)
	$(CXX) -o $@ $^ $(LIB) 

example/fm-synth: example/fm-synth.o $(OBJ)
	$(CXX) -o $@ $^ $(LIB) 

clean:
	rm -f $(OBJ)
	rm -f app
	rm -f app.o
	rm -f example/formant-synth
	rm -f example/formant-synth.o
	rm -f example/fm-synth
	rm -f example/fm-synth.o
