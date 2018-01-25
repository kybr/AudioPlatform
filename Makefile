# Karl Yerkes
# karl.yerkes@gmail.com
# 2017-11-03
# MAT 240C
#
#
.SUFFIXES:

TARGET = app

CXX=
CXX += c++
CXX += -std=c++11
CXX += -Wall
CXX += -g
#CXX += -D__LINUX_ALSA__ # if you're using linux
CXX += -D__MACOSX_CORE__

INC=
INC += -I/usr/local/include/
INC += -Irtaudio/
INC += -Irtmidi/
INC += -IAudioFile/
INC += -Iimgui/
INC += -Iimgui/examples/libs/gl3w/
INC += -Iimgui/examples/libs/gl3w/
INC += -Iimgui/examples/opengl3_example/

LIB=
LIB += -L/usr/local/lib/
LIB += -lpthread
LIB += -lglfw

# these are macOS-specific
LIB += -framework CoreFoundation
LIB += -framework CoreAudio
LIB += -framework CoreMIDI

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $<

%.o: %.c
	cc $(INC) -c -o $@ $<

app: app.o AudioFile/AudioFile.o rtaudio/RtAudio.o rtmidi/RtMidi.o imgui/examples/libs/gl3w/GL/gl3w.o imgui/examples/opengl3_example/imgui_impl_glfw_gl3.o imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
	$(CXX) $(LIB) -o $@ $^

clean:
	rm app *.o
