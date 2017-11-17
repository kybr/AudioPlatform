#ifndef __240C_SYNTHS__
#define __240C_SYNTHS__

struct Phasor {
  float phase = 0.0f, increment = 0.0f;
  void frequency(float f) { increment = f / sampleRate; }
  virtual void trigger() {}
  virtual float nextValue() {
    float returnValue = phase;
    phase += increment;
    // wrapping phase
    if (phase > 1.0f) {
      trigger();
      phase -= 1.0f;
    };
    return returnValue;
  }
};

struct FloatArray {
  float* data = nullptr;
  unsigned size = 0;

  virtual ~FloatArray() {
    if (data != nullptr) delete[] data;
  }

  float operator[](unsigned index) { return data[index]; }

  // a way to resize
  void zeros(unsigned n) {
    if (data != nullptr) delete[] data;  // or your have a memory leak
    data = new float[n];
    size = n;
    for (unsigned i = 0; i < n; ++i) data[i] = 0.0f;
  }
};

struct FloatArrayWithLinearInterpolation : FloatArray {
  float operator[](const float index) const {
    const unsigned i = floor(index);
    const float x0 = data[i];
    const float x1 = data[(i == (size - 1)) ? 0 : i];  // looping semantics
    const float t = index - i;
    return x0 * t + x1 * (1 - t);
  }
};

struct Table {
  float phase = 0.0f;
  float increment = 0.0f;
  float* data;
  unsigned size;

  Table(float* data = nullptr, unsigned size = 4096) : data(data), size(size) {}

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
};

struct Noise : Table {
  Noise(unsigned size = 10000) {
    this->size = size;
    data = new float[size];
    for (unsigned i = 0; i < size; ++i)
      data[i] = 2.0f * (random() / float(RAND_MAX)) - 1.0f;
  }
};

struct Sine : Table {
  Sine(unsigned size = 10000) {
    this->size = size;
    data = new float[size];
    const float pi2 = M_PI * 2;
    for (unsigned i = 0; i < size; ++i) data[i] = sinf(i * pi2 / size);
  }
};

struct Timer {
  float phase = 0.0f, increment = 0.0f;
  void frequency(float f) { increment = f / sampleRate; }
  void period(float p) { increment = 1.0f / (p * sampleRate); }
  void ms(float p) { period(p / 1000.0f); }
  bool hasFired() {
    phase += increment;
    if (phase > 1.0f) {
      phase -= 1.0f;
      return true;
    }
    return false;
  }
};

struct Line {
  float target, value, milliseconds, increment;

  Line() { set(0.0f, 0.0f, 30.0f); }

  void set() {
    increment = (target - value) / (sampleRate * (milliseconds / 1000.0f));
  }
  void set(float value, float target, float milliseconds) {
    this->value = value;
    this->target = target;
    this->milliseconds = milliseconds;
    set();  // sets increment based on the above
  }
  void set(float target, float milliseconds) {
    set(value, target, milliseconds);
  }
  void set(float target) { set(value, target, milliseconds); }

  bool done() {
    if (value == target) return true;
    // if the increment is negative, then we're done when the value is lower
    // than the target. alternatively, if the increment is positive, then
    // we're done when the value is greater than the target. in both cases, we
    // detect overshoot.
    return (increment < 0) ? (value < target) : (value > target);
  }

  float nextValue() {
    float returnValue = value;
    if (done())
      value = target;
    else
      value += increment;
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

struct ADSR {
  float a, d, s, r;
  Line attack, decay, release;
  int state;
  bool loop = false;

  ADSR() { set(300.0f, 200.0f, 0.7f, 500.0f); }

  void reset() {
    state = 0;
    attack.set(0.0f, 1.0f, a);  // value, target, milliseconds
    decay.set(1.0f, s, d);
    release.set(s, 0.0f, r);
  }

  void set(float a, float d, float s, float r) {
    this->a = a;
    this->d = d;
    this->s = s;
    this->r = r;
    reset();
  }

  float nextValue() {
    switch (state) {
      case 0:
        if (attack.done()) state = 1;
        break;
      case 1:
        if (decay.done()) state = 2;
        break;
      case 2:
        if (release.done()) state = 3;
        break;
      default:
        if (loop) reset();
        break;
    }
    switch (state) {
      case 0:
        // cout << "a:";
        return attack.nextValue();
      case 1:
        // cout << "d:";
        return decay.nextValue();
      case 2:
        // cout << "r:";
        return release.nextValue();
      default:
        // we are spent; call trigger to start again
        // cout << "e:";
        return 0.0f;
    }
  }
};

#endif
