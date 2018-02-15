#ifndef __AP_GLOBAL__
#define __AP_GLOBAL__

#define die(message, ...)                                               \
  do {                                                                  \
    fprintf(stderr, "died in %s at line %d with ", __FILE__, __LINE__); \
    fprintf(stderr, message, ##__VA_ARGS__);                            \
    fprintf(stderr, "\n");                                              \
    exit(-1);                                                           \
  } while (0);

#define info(message, ...)                                          \
  do {                                                              \
    fprintf(stderr, "info in %s at line %d: ", __FILE__, __LINE__); \
    fprintf(stderr, message, ##__VA_ARGS__);                        \
    fprintf(stderr, "\n");                                          \
  } while (0);

namespace ap {

static unsigned channelCount = 2;
static float sampleRate = 44100.0f;
static unsigned blockSize = 512;

}  // namespace ap

#endif
