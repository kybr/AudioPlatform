#ifndef __240C_SYNTHS__
#define __240C_SYNTHS__

#include <cmath>
#include <string>
#include "Wav.h"

namespace ap {

void normalize(float* data, unsigned size) {
  float max = 0;
  for (unsigned i = 0; i < size; ++i)
    if (max < abs(data[i])) max = data[i];
  for (unsigned i = 0; i < size; ++i) data[i] /= max;
}

struct Timer {
  float phase = 0.0f, increment = 0.0f;
  void period(float s) { increment = 1.0f / (s * sampleRate); }
  void frequency(float hz) { period(1 / hz); }

  virtual bool operator()() { return nextValue(); }
  virtual bool nextValue() {
    phase += increment;
    if (phase > 1.0f) {
      phase -= 1.0f;
      return true;
    };
    if (phase < 0.0f) {
      phase += 1.0f;
      return true;
    };
    return false;
  }
};

struct Phasor {
  float phase = 0.0f, increment = 0.0f;
  void frequency(float hz) { increment = hz / sampleRate; }
  void period(float s) { frequency(1 / s); }
  virtual float operator()() { return nextValue(); }
  virtual float nextValue() {
    float returnValue = phase;
    phase += increment;
    if (phase > 1.0f) phase -= 1.0f;
    if (phase < 0.0f) phase += 1.0f;  // sure; handle negative frequency
    return returnValue;
  }
};

struct Array {
  float* data = nullptr;
  unsigned size = 0;

  virtual ~Array() {
    if (data) delete[] data;
  }

  float& operator[](unsigned index) { return data[index]; }
  float operator[](const float index) const { return get(index); }

  void resize(unsigned n) {
    size = n;
    if (data) delete[] data;  // or your have a memory leak
    if (n == 0) {
      data = nullptr;
    } else {
      data = new float[n];
      for (unsigned i = 0; i < n; ++i) data[i] = 0.0f;
    }
  }

  float get(const float index) const {
    const unsigned i = floor(index);
    const float x0 = data[i];
    const float x1 = data[(i == (size - 1)) ? 0 : i + 1];  // looping semantics
    const float t = index - i;
    return x1 * t + x0 * (1 - t);
  }

  void add(const float index, const float value) {
    const unsigned i = floor(index);
    const unsigned j = (i == (size - 1)) ? 0 : i + 1;  // looping semantics
    const float t = index - i;
    data[i] += value * (1 - t);
    data[j] += value * t;
  }
};

struct Table : Phasor, Array {
  Table(unsigned size = 4096) { resize(size); }

  float operator()() { return nextValue(); }
  float nextValue() {
    const float v = get(phase * size);
    Phasor::nextValue();
    return v;
  }
};

struct Noise : Table {
  Noise(unsigned size = 10000) {
    resize(size);
    for (unsigned i = 0; i < size; ++i)
      data[i] = 2.0f * (random() / float(RAND_MAX)) - 1.0f;

    normalize(data, size);
  }
};

struct Sine : Table {
  Sine(unsigned size = 10000) {
    const float pi2 = M_PI * 2;
    resize(size);
    for (unsigned i = 0; i < size; ++i) data[i] = sinf(i * pi2 / size);
  }
};

struct SamplePlayer : Table {
  float playbackRate;

  void load(std::string filePath, unsigned channel = 0) {
    unsigned int channelCount;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;

    float* pSampleData = drwav_open_and_read_file_f32(
        filePath.c_str(), &channelCount, &sampleRate, &totalSampleCount);

    if (pSampleData == NULL) {
      printf("ERROR\n");
      exit(1);
    }

    //
    playbackRate = sampleRate;
    resize(totalSampleCount);
    //
    for (unsigned i = 0; i < size; ++i)
      data[i] = pSampleData[channelCount * i + channel];

    frequency(1.0f);

    printf("%s -> %d samples @ %f Hz\n", filePath.c_str(), size, playbackRate);
  }

  void save(std::string filePath = "out.wav") {
    drwav_data_format format;
    format.channels = 1;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.sampleRate = 44100;  // XXX hack for now
    format.bitsPerSample = 32;
    drwav* pWav = drwav_open_file_write(filePath.c_str(), &format);
    drwav_uint64 samplesWritten = drwav_write(pWav, size, &data[0]);
    drwav_close(pWav);
    assert(samplesWritten == size);
  }

  void frequency(float f) { Phasor::frequency(f * playbackRate / size); }
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

  float operator()() { return nextValue(); }
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
  float operator()() { return nextValue(); }
};

struct Square : Phasor {
  float nextValue() { return Phasor::nextValue() > 0.5f ? 1.0f : -1.0f; }
  float operator()() { return nextValue(); }
};

struct Impulse : Phasor {
  float previousOutput;
  float nextValue() {
    float f = Phasor::nextValue() < previousOutput ? 1.0f : 0.0f;
    previousOutput = f;
    return f;
  }
  float operator()() { return nextValue(); }
};

struct Triangle : Saw {
  float nextValue() {
    float f = Phasor::nextValue() * 4.0f - 2.0f;
    return (f > 1.0f) ? 2.0f - f : ((f < -1.0f) ? -2.0f - f : f);
  }
  float operator()() { return nextValue(); }
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
  float operator()() { return nextValue(); }

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
  float b0, b1, b2, a1, a2;

 public:
  float operator()(float x0) {
    // Direct Form 1, normalized...
    float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    y2 = y1;
    y1 = y0;
    x2 = x1;
    x1 = x0;
    return y0;
  }

  Biquad() { apf(1000.0f, 0.5f); }

  void normalize(float a0) {
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
    // print();
  }

  void print() {
    printf("b0:%f ", b0);
    printf("b1:%f ", b1);
    printf("b2:%f ", b2);
    printf("a1:%f ", a1);
    printf("a2:%f ", a2);
    printf("\n");
  }

  void lpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = (1 - cos(w0)) / 2;
    b1 = 1 - cos(w0);
    b2 = (1 - cos(w0)) / 2;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }

  void hpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = (1 + cos(w0)) / 2;
    b1 = -(1 + cos(w0));
    b2 = (1 + cos(w0)) / 2;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }

  void bpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = Q * alpha;
    b1 = 0;
    b2 = -Q * alpha;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }

  void notch(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = 1;
    b1 = -2 * cos(w0);
    b2 = 1;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }

  void apf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    b0 = 1 - alpha;
    b1 = -2 * cos(w0);
    b2 = 1 + alpha;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }
};

// actually an ADR
struct ADSR {
  float a, d, s, r;
  Line attack, decay, release;
  int state;
  bool loop = false;

  ADSR() { set(50.0f, 100.0f, 0.7f, 300.0f); }

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

  float operator()() { return nextValue(); }
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

class BiquadWithLines {
  // Audio EQ Cookbook
  // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

  // x[n-1], x[n-2], y[n-1], y[n-2]
  float x1, x2, y1, y2;

  // filter coefficients
  Line b0, b1, b2, a1, a2;

 public:
  float operator()(float x0) {
    // Direct Form 1, normalized...
    float y0 = b0() * x0 + b1() * x1 + b2() * x2 - a1() * y1 - a2() * y2;
    y2 = y1;
    y1 = y0;
    x2 = x1;
    x1 = x0;
    return y0;
  }

  BiquadWithLines() { lpf(10000.0f, 0.7f); }

  void lpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    float a0 = 1 + alpha;
    b0.set(((1 - cos(w0)) / 2) / a0);
    b1.set((1 - cos(w0)) / a0);
    b2.set(((1 - cos(w0)) / 2) / a0);
    a1.set((-2 * cos(w0)) / a0);
    a2.set((1 - alpha) / a0);
  }

  void hpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    float a0 = 1 + alpha;
    b0.set(((1 + cos(w0)) / 2) / a0);
    b1.set((-(1 + cos(w0))) / a0);
    b2.set(((1 + cos(w0)) / 2) / a0);
    a1.set((-2 * cos(w0)) / a0);
    a2.set((1 - alpha) / a0);
  }

  void bpf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    float a0 = 1 + alpha;
    b0.set((Q * alpha) / a0);
    b1.set((0) / a0);
    b2.set((-Q * alpha) / a0);
    a1.set((-2 * cos(w0)) / a0);
    a2.set((1 - alpha) / a0);
  }

  void notch(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    float a0 = 1 + alpha;
    b0.set((1) / a0);
    b1.set((-2 * cos(w0)) / a0);
    b2.set((1) / a0);
    a1.set((-2 * cos(w0)) / a0);
    a2.set((1 - alpha) / a0);
  }

  void apf(float f0, float Q) {
    float w0 = 2 * M_PI * f0 / sampleRate;
    float alpha = sin(w0) / (2 * Q);
    float a0 = 1 + alpha;
    b0.set((1 - alpha) / a0);
    b1.set((-2 * cos(w0)) / a0);
    b2.set((1 + alpha) / a0);
    a1.set((-2 * cos(w0)) / a0);
    a2.set((1 - alpha) / a0);
  }
};

}  // namespace ap

#endif
