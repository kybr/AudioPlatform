#include <cassert>
#include <iostream>
#include "AudioPlatform/Wav.h"

int main(int argc, char* argv[]) {
  assert(argc == 2);
  unsigned int channelCount;
  unsigned int sampleRate;
  drwav_uint64 totalSampleCount;

  float* pSampleData = drwav_open_and_read_file_f32(
      argv[1], &channelCount, &sampleRate, &totalSampleCount);

  if (pSampleData == NULL) {
    printf("ERROR\n");
    exit(1);
  }

  assert(sampleRate == 44100);
  assert(channelCount == 1);
  for (unsigned i = 0; i < totalSampleCount; ++i)
    printf("%f\n", pSampleData[i]);
}
