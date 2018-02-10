#ifndef __AP_MIDI__
#define __AP_MIDI__

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
