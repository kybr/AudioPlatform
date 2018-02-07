#ifndef __240C_AUDIO__
#define __240C_AUDIO__

#include "rtaudio/RtAudio.h"

namespace ap {

const unsigned channelCount = 2;
const float sampleRate = 44100.0f;
const unsigned blockSize = 512;

int cb(void *, void *, unsigned int, double, RtAudioStreamStatus, void *);

struct Audio {
  RtAudio dac;

  void start() {
  //RtAudio::Api getCurrentApi() { return RtAudio::LINUX_ALSA; }

    //if (dac.getDeviceCount() < 1) {
    //  std::cout << "\nNo audio devices found!\n";
    //  exit(1);
    //}

    dac.showWarnings(true);

    RtAudio::StreamParameters oParams;
    oParams.deviceId = 0;
    oParams.nChannels = channelCount;
    oParams.firstChannel = 0;
    oParams.deviceId = dac.getDefaultOutputDevice();

    RtAudio::StreamOptions options;
    // options.flags = RTAUDIO_HOG_DEVICE;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    try {
      unsigned bs = blockSize;
      dac.openStream(&oParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bs, &cb,
                     (void *)this, &options, nullptr);
      if (bs != blockSize) {
        printf("FAIL");
        exit(2);
      }
      dac.startStream();
    } catch (RtAudioError &e) {
      e.printMessage();
      if (dac.isStreamOpen()) dac.closeStream();
      exit(1);
    }
  }

  ~Audio() {
    try {
      dac.stopStream();
    } catch (RtAudioError &e) {
      e.printMessage();
    }
  }

  virtual void audio(float *out) {
    for (unsigned i = 0; i < blockSize * channelCount; i += channelCount) {
      out[i + 0] = 0;
      out[i + 1] = 0;
    }
  }
};

int cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
       double streamTime, RtAudioStreamStatus status, void *data) {
  ((Audio *)data)->audio((float *)outputBuffer);
  if (status) std::cout << "Stream underflow detected!" << std::endl;
  return 0;
}

}  // namespace ap

#endif
