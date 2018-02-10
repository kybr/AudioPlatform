#ifndef __240C_MIDI__
#define __240C_MIDI__

#include <vector>
#include "rtmidi/RtMidi.h"

namespace ap {

struct MIDI {
  RtMidiIn *midiin = nullptr;
  void setup();
  void receive(std::vector<unsigned char> &message);
};

}  // namespace ap

#endif
