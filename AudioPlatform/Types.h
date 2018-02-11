#ifndef __AP_TYPES__
#define __AP_TYPES__

namespace ap {

struct Array {
  float* data = nullptr;
  unsigned size = 0;

  virtual ~Array();

  float& operator[](unsigned index);
  float operator[](const float index) const;

  void resize(unsigned n);
  void zeros(unsigned n);

  float get(const float index) const;

  void add(const float index, const float value);
};

typedef Array FloatArrayWithLinearInterpolation;

}  // namespace ap

#endif
