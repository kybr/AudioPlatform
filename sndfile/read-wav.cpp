#include <cassert>
#include <iostream>
#include <sndfile.hh>

int main(int argc, char* argv[]) {
  assert(argc == 2);
  SndfileHandle file = SndfileHandle(argv[1]);
  if (!file.formatCheck(SF_FORMAT_WAV | SF_FORMAT_FLOAT, 1, 44100)) {
    fprintf(stderr, "ERROR!\n");
    return 1;
  }
  float* data = new float[file.frames()];
  file.read(data, file.frames());
  for (int i = 0; i < file.frames(); ++i) printf("%f\n", data[i]);

  return 0;
}
