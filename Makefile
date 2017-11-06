# Karl Yerkes
# karl.yerkes@gmail.com
# 2017-11-03
# MAT 240C
#
#
TARGET = app

COMPILER += c++
COMPILER += -std=c++11
COMPILER += -Wall
COMPILER += -g
COMPILER += -D__MACOSX_CORE__

INCLUDES += -I/usr/local/include/
INCLUDES += -Irtaudio/
INCLUDES += -Iimgui/
INCLUDES += -Iimgui/examples/libs/gl3w/
INCLUDES += -Iimgui/examples/libs/gl3w/
INCLUDES += -Iimgui/examples/opengl3_example/

LIBRARIES += -L/usr/local/lib/
LIBRARIES += -lpthread
LIBRARIES += -framework CoreFoundation
LIBRARIES += -framework CoreAudio
LIBRARIES += -lglfw

_:
	$(COMPILER) -c $(INCLUDES) $(TARGET).cpp
	$(COMPILER) -c $(INCLUDES) rtaudio/RtAudio.cpp
	clang -x c -c $(INCLUDES) imgui/examples/libs/gl3w/GL/gl3w.c
	$(COMPILER) -c $(INCLUDES) imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp
	$(COMPILER) -c $(INCLUDES) imgui/imgui.cpp
	$(COMPILER) -c $(INCLUDES) imgui/imgui_demo.cpp
	$(COMPILER) -c $(INCLUDES) imgui/imgui_draw.cpp
	$(COMPILER) -o $(TARGET) *.o $(LIBRARIES)

clean:
	rm -f $(TARGET)
	rm -f *.o
