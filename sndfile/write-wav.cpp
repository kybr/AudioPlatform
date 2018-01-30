#include <iostream>    // atof
#include <sndfile.hh>  // SndfileHandle, SFM_WRITE, SF_FORMAT_WAV, ..
#include <vector>

int main(int argc, char* argv[]) {
  std::vector<float> y;

  std::string line;
  getline(std::cin, line);
  while (line != "") {
    y.push_back(atof(line.c_str()));
    getline(std::cin, line);
  }

  {
    SndfileHandle file = SndfileHandle(
        "out.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, 1, 44100);
    file.write(&y[0], y.size());
  }
  return 0;
}
