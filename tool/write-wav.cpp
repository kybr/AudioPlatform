#include <iostream>
#include <vector>
#include "AudioPlatform/Wav.h"

int main(int argc, char* argv[]) {
  std::vector<float> y;

  std::string line;
  getline(std::cin, line);
  while (line != "") {
    y.push_back(atof(line.c_str()));
    getline(std::cin, line);
  }

  {
    drwav_data_format format;
    format.channels = 1;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.sampleRate = 44100;
    format.bitsPerSample = 32;
    drwav* pWav = (argc == 2) ? drwav_open_file_write(argv[1], &format)
                              : drwav_open_file_write("out.wav", &format);
    drwav_uint64 samplesWritten = drwav_write(pWav, y.size(), &y[0]);
    drwav_close(pWav);
    assert(samplesWritten == y.size());
  }
}
