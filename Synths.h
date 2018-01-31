#ifndef __240C_SYNTHS__
#define __240C_SYNTHS__

// Timer t;
// t.period(1);
// // ...
// for each sample
// ...
//   if (t()) {
//     // reset an oscillator
//   }
//
struct Timer {
  float phase = 0.0f, increment = 0.0f;
  // Karl, what are the units?
  void period(float t) { increment = 1.0f / (t * sampleRate); }
  // void period(float t) { increment = (1.0f / t) / sampleRate; }

  virtual bool operator()() {
    phase += increment;
    if (phase > 1.0f) {
      phase -= 1.0f;
      return true;
    };
    // if you happen to be running a timer backwards!? necessary?
    if (phase < 0.0f) {
      phase += 1.0f;
      return true;
    };
    return false;
  }
};

// Phasor p;
// p.frequency(220)
// ...
// float s = p(); // (0, 1)
struct Phasor {
  Timer timer;
  void frequency(float f) { timer.period(1.0f / f); }

  virtual void trigger() {}
  virtual float operator()() { return nextValue(); }
  virtual float nextValue() {
    float returnValue = timer.phase;
    if (timer()) trigger();
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
    size = n;
    if (data != nullptr) delete[] data;  // or your have a memory leak
    if (n == 0) {
      data = nullptr;
    } else {
      data = new float[n];
      for (unsigned i = 0; i < n; ++i) data[i] = 0.0f;
    }
  }
};

struct FloatArrayWithLinearInterpolation : FloatArray {
  float operator[](const float index) const { return get(index); }
  float get(const float index) const {
    const unsigned i = floor(index);
    const float x0 = data[i];
    const float x1 = data[(i == (size - 1)) ? 0 : i + 1];  // looping semantics
    const float t = index - i;
    return x1 * t + x0 * (1 - t);
  }
};

struct Table : Phasor, FloatArrayWithLinearInterpolation {
  Table(unsigned size = 4096) { zeros(size); }

  float operator()() { return nextValue(); }
  float nextValue() {
    const float v = get(timer.phase * size);
    Phasor::nextValue();
    return v;
  }
};

struct Noise : Table {
  Noise(unsigned size = 10000) {
    zeros(size);
    for (unsigned i = 0; i < size; ++i)
      data[i] = 2.0f * (random() / float(RAND_MAX)) - 1.0f;
  }
};

struct Sine : Table {
  Sine(unsigned size = 10000) {
    const float pi2 = M_PI * 2;
    zeros(size);
    for (unsigned i = 0; i < size; ++i) data[i] = sinf(i * pi2 / size);
  }
};

struct SamplePlayer : Table {
  float playbackRate;
  SamplePlayer(std::string filePath) {
    // https://github.com/adamstark/AudioFile
    AudioFile<float> audioFile;
    assert(audioFile.load(filePath));
    playbackRate = audioFile.getSampleRate();
    assert(audioFile.getNumSamplesPerChannel() > 0);
    zeros(audioFile.getNumSamplesPerChannel());
    for (int i = 0; i < size; ++i) data[i] = audioFile.samples[0][i];
    frequency(1.0f);

    printf("%s -> %d samples @ %f Hz\n", filePath.c_str(), size, playbackRate);
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
    if (sync) other->timer.phase = 0.0f;
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

struct QuasiBandlimited {
  //
  // "Synthesis of Quasi-Bandlimited Analog Waveforms Using Frequency
  // Modulation"
  //   by Peter Schoffhauzer
  // (http://scp.web.elte.hu/papers/synthesis1.pdf)
  //
  const float a0 = 2.5;   // precalculated coeffs
  const float a1 = -1.5;  // for HF compensation

  // variables
  float osc;      // output of the saw oscillator
  float osc2;     // output of the saw oscillator 2
  float phase;    // phase accumulator
  float w;        // normalized frequency
  float scaling;  // scaling amount
  float DC;       // DC compensation
  float norm;     // normalization amount
  float last;     // delay for the HF filter

  float Frequency, Filter, PulseWidth;

  QuasiBandlimited() {
    reset();
    Frequency = 1.0;
    Filter = 1.0;
    PulseWidth = 0.5;
    recalculate();
  }

  void reset() {
    // zero oscillator and phase
    osc = 0.0;
    osc2 = 0.0;
    phase = 0.0;
  }

  void recalculate() {
    w = Frequency / sampleRate;  // normalized frequency
    float n = 0.5 - w;
    scaling = Filter * 13.0f * powf(n, 4.0f);  // calculate scaling
    DC = 0.376 - w * 0.752;                    // calculate DC compensation
    norm = 1.0 - 2.0 * w;                      // calculate normalization
  }

  void frequency(float f) {
    Frequency = f;
    recalculate();
  }

  void filter(float f) {
    Filter = f;
    recalculate();
  }

  void pulseWidth(float w) {
    PulseWidth = w;
    recalculate();
  }

  void step() {
    // increment accumulator
    phase += 2.0 * w;
    if (phase >= 1.0) phase -= 2.0;
    if (phase <= -1.0) phase += 2.0;
  }

  // process loop for creating a bandlimited saw wave
  float saw() {
    step();

    // calculate next sample
    osc = (osc + sinf(2 * M_PI * (phase + osc * scaling))) * 0.5;
    // compensate HF rolloff
    float out = a0 * osc + a1 * last;
    last = osc;
    out = out + DC;     // compensate DC offset
    return out * norm;  // store normalized result
  }

  // process loop for creating a bandlimited PWM pulse
  float pulse() {
    step();

    // calculate saw1
    osc = (osc + sinf(2 * M_PI * (phase + osc * scaling))) * 0.5;
    // calculate saw2
    osc2 =
        (osc2 + sinf(2 * M_PI * (phase + osc2 * scaling + PulseWidth))) * 0.5;
    float out = osc - osc2;  // subtract two saw waves
    // compensate HF rolloff
    out = a0 * out + a1 * last;
    last = osc;
    return out * norm;  // store normalized result
  }
};

#endif
