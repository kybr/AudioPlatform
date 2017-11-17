#ifndef __240C_SYNTHS__
#define __240C_SYNTHS__

class Table {
  Table() {}
  float phase = 0.0f;
  float increment = 0.0f;
  float* data;
  unsigned size;
  Table(float* data, unsigned size) : data(data), size(size) {}

 public:
  void frequency(float f) { increment = f / sampleRate; }
  float operator()() {
    const float _phase = phase * size;
    const unsigned index = floor(_phase);
    const float x0 = data[index];
    const float x1 = data[(index == (size - 1)) ? 0 : index];
    const float t = _phase - index;
    const float v = x0 * t + x1 * (1 - t);
    phase += increment;
    if (phase > 1.0f) phase -= 1.0f;
    return v;
  }
  static Table* makeSine(unsigned size = 10000) {
    float* data = new float[size];
    const float pi2 = M_PI * 2;
    for (unsigned i = 0; i < size; ++i) data[i] = sinf(i * pi2 / size);
    return new Table(data, size);
  }
};

struct Timer {
  float phase = 0.0f, phase_increment = 0.0f;
  void frequency(float f) { phase_increment = f / sampleRate; }
  void period(float p) { phase_increment = 1.0f / (p * sampleRate); }
  void ms(float p) { period(p / 1000.0f); }
  bool hasFired() {
    phase += phase_increment;
    if (phase > 1.0f) {
      phase -= 1.0f;
      return true;
    }
    return false;
  }
};

struct Phasor {
  float phase = 0.0f, phase_increment = 0.0f;
  void frequency(float f) { phase_increment = f / sampleRate; }
  virtual void trigger() {}
  virtual float nextValue() {
    float returnValue = phase;
    phase += phase_increment;
    // wrapping phase
    if (phase > 1.0f) {
      trigger();
      phase -= 1.0f;
    };
    return returnValue;
  }
};

struct Saw : Phasor {
  float nextValue() { return Phasor::nextValue() * 2.0f - 1.0f; }
};

struct Square : Phasor {
  float nextValue() { return Phasor::nextValue() > 0.5f ? 1.0f : -1.0f; }
};

struct Impulse : Phasor {
  float previousOutput;
  float nextValue() {
    float f = Phasor::nextValue() < previousOutput ? 1.0f : 0.0f;
    previousOutput = f;
    return f;
  }
};

struct Triangle : Saw {
  float nextValue() {
    float f = Phasor::nextValue() * 4.0f - 2.0f;
    return (f > 1.0f) ? 2.0f - f : ((f < -1.0f) ? -2.0f - f : f);
  }
};

struct MultiSynth : Phasor {
  int type = 0;

  float nextValue() {
    switch (type) {
      default:
      case 0:
        return saw();
      case 1:
        return triangle();
      case 2:
        return square();
      case 3:
        return impulse();
    }
  }

  float square() { return Phasor::nextValue() > 0.5f ? 1.0f : -1.0f; }

  float saw() { return Phasor::nextValue() * 2.0f - 1.0f; }

  float triangle() {
    float f = Phasor::nextValue() * 4.0f - 2.0f;
    return (f > 1.0f) ? 2.0f - f : ((f < -1.0f) ? -2.0f - f : f);
  }

  float impulse() {
    static float previousPhase = 0.0f;
    float f = Phasor::nextValue();
    float returnValue = f < previousPhase ? 1.0f : 0.0f;
    previousPhase = f;
    return returnValue;
  }
};

struct HardSyncMultiSynth : MultiSynth {
  bool sync = false;
  MultiSynth* other = nullptr;
  void trigger() {
    if (other == nullptr) return;
    if (sync) other->phase = 0.0f;
    // if (sync) other->phase = 0.999999f;
  }
};

class Biquad {
  // Audio EQ Cookbook
  // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

  // x[n-1], x[n-2], y[n-1], y[n-2]
  float x1, x2, y1, y2;

  // filter coefficients
  float a0, a1, a2;
  float b0, b1, b2;

 public:
  float operator()(float x0) {
    float y0 = (b0 / a0) * x0 + (b1 / a0) * x1 + (b2 / a0) * x2 -
               (a1 / a0) * y1 - (a2 / a0) * y2;
    y2 = y1;
    y1 = y0;
    x2 = x1;
    x1 = x0;
    return y0;
  }

  Biquad() { lpf(1000.0f, 0.5f); }

  void lpf(float f0, float Q) {
    float w0 = 2 * pi * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = (1 - cos(w0)) / 2;
    b1 = 1 - cos(w0);
    b2 = (1 - cos(w0)) / 2;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;
  }

  void hpf(float f0, float Q) {
    float w0 = 2 * pi * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = (1 + cos(w0)) / 2;
    b1 = -(1 + cos(w0));
    b2 = (1 + cos(w0)) / 2;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;
  }

  void bpf(float f0, float Q) {
    float w0 = 2 * pi * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = Q * alpha;
    b1 = 0;
    b2 = -Q * alpha;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;
  }

  void notch(float f0, float Q) {
    float w0 = 2 * pi * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = 1;
    b1 = -2 * cos(w0);
    b2 = 1;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;
  }

  void apf(float f0, float Q) {
    float w0 = 2 * pi * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = 1 - alpha;
    b1 = -2 * cos(w0);
    b2 = 1 + alpha;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;
  }
};

#endif
