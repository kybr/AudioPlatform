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
INC += -Irtaudio/
INC += -Irtmidi/
INC += -IAudioFile/
INC += -Iimgui/
INC += -Iimgui/examples/libs/gl3w/
INC += -Iimgui/examples/opengl3_example/

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

TARGET = app

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $<

%.o: %.c
	cc $(INC) -c -o $@ $<

app: app.o AudioFile/AudioFile.o rtaudio/RtAudio.o rtmidi/RtMidi.o imgui/examples/libs/gl3w/GL/gl3w.o imgui/examples/opengl3_example/imgui_impl_glfw_gl3.o imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
	$(CXX) -o $@ $^ $(LIB) 

clean:
	rm imgui/examples/libs/gl3w/GL/gl3w.o
	rm imgui/examples/opengl3_example/imgui_impl_glfw_gl3.o
	rm imgui/imgui_draw.o
	rm imgui/imgui_demo.o
	rm imgui/imgui.o
	rm rtmidi/RtMidi.o
	rm app.o
	rm AudioFile/AudioFile.o
	rm rtaudio/RtAudio.o
