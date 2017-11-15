#ifndef __240C_SYNTHS__
#define __240C_SYNTHS__

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

#endif
