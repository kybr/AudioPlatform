.SUFFIXES:

CXX  = c++
CXX += -std=c++11
CXX += -Wall
CXX += -g
INC += -I /usr/local/include
LIB += -L /usr/local/lib
LIB += -l sndfile

_: read-wav write-wav sine sawtooth sine-sweep additive-synth

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $<

read-wav: read-wav.o
	$(CXX)  -o $@ $^ $(LIB)

write-wav: write-wav.o
	$(CXX)  -o $@ $^ $(LIB)

sine: sine.o
	$(CXX)  -o $@ $^ $(LIB)

sawtooth: sawtooth.o
	$(CXX)  -o $@ $^ $(LIB)

sine-sweep: sine-sweep.o
	$(CXX)  -o $@ $^ $(LIB)

additive-synth: additive-synth.o
	$(CXX)  -o $@ $^ $(LIB)

clean:
	rm additive-synth read-wav write-wav sine sawtooth sine-sweep *.o
