#ifndef __240C_MIDI__
#define __240C_MIDI__

#include "RtMidi.h"

struct MIDI {
  RtMidiIn *midiin = nullptr;

  MIDI() {
    try {
      midiin = new RtMidiIn();
    } catch (RtMidiError &error) {
      error.printMessage();
      exit(1);
    }

    unsigned int nPorts = midiin->getPortCount();
    std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
    std::string portName;
    for (unsigned int i = 0; i < nPorts; i++) {
      try {
        portName = midiin->getPortName(i);
      } catch (RtMidiError &error) {
        error.printMessage();
        exit(1);
      }
      std::cout << "  Input Port #" << i + 1 << ": " << portName << '\n';
    }

    try {
      unsigned int port = 0;
      std::cout << "Attempting to open port 0 (first midi port)" << std::endl;
      midiin->openPort(port);
    } catch (RtMidiError &error) {
      error.printMessage();
      exit(1);
    }

    // Don't ignore sysex, timing, or active sensing messages.
    //
    midiin->ignoreTypes(false, false, false);
  }

  void midi(std::vector<unsigned char> &message) {
    double stamp = midiin->getMessage(&message);
    int nBytes = message.size();
    for (int i = 0; i < nBytes; i++)
      std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
    if (nBytes > 0) std::cout << "stamp = " << stamp << std::endl;
  }
};

#endif
