#ifndef __AP_WINDOWS__
#define __AP_WINDOWS__

#include "AudioPlatform/Types.h"

#include <vector>

namespace ap {

void hann(Array& window, unsigned size);
void hann(std::vector<float>& window, unsigned size);
float mtof(float m);
float ftom(float f);
float dbtoa(float db);
float atodb(float a);
void normalize(float* data, unsigned size);
float uniform(float low, float high);
float uniform(float high = 1.0f);
float map(float value, float low, float high, float low_, float high_);
float distortion(float f, float t);

}  // namespace ap
#endif
