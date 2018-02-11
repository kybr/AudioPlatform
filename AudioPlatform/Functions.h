#ifndef __AP_WINDOWS__
#define __AP_WINDOWS__

#include "AudioPlatform/Types.h"

namespace ap {

void hanning(Array& window, unsigned size);
float mtof(float m);
float ftom(float f);
float dbtoa(float db);
float atodb(float a);

}  // namespace ap
#endif
