#include <cmath>
#include <iostream>
#include <vector>
#include "AudioPlatform/Synths.h"

using namespace std;
using namespace ap;

void convolve(const vector<float> &f, const vector<float> &g,
              vector<float> &fg) {
  fg.resize(f.size() + g.size() - 1, 0);

  // convolution process
  for (unsigned i = 0; i < fg.size(); i++) {
    int i1 = i;
    float tmp = 0.0;
    for (unsigned j = 0; j < g.size(); j++) {
      if (i1 >= 0 && i1 < (int)f.size())
        // accumulate
        tmp = tmp + (f[i1] * g[j]);
      i1 = i1 - 1;
      fg[i] = tmp;
    }
    cout << i << " done of " << fg.size() << " is " << fg[i] << endl;
  }
}

void fill(float *data, unsigned size, vector<float> &out) {
  out.resize(size);
  for (unsigned i = 0; i < out.size(); ++i) out[i] = data[i];
}

void normalize(vector<float> &a) {
  float max = 0;
  for (unsigned i = 0; i < a.size(); ++i)
    if (abs(a[i]) > max) max = abs(a[i]);
  for (unsigned i = 0; i < a.size(); ++i) a[i] /= max;
}

int main(int argc, char *argv[]) {
  SamplePlayer in1, in2;
  in1.load(argv[1]);
  in2.load(argv[2]);

  vector<float> a, b;
  fill(in1.data, in1.size, a);
  fill(in2.data, in2.size, b);
  cout << a.size() << " " << b.size() << endl;
  //
  //
  vector<float> c;
  convolve(a, b, c);
  normalize(c);
  SamplePlayer out;
  out.resize(c.size());
  for (unsigned i = 0; i < c.size(); ++i) out.data[i] = c[i];
  out.save();
}
