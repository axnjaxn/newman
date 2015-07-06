#ifndef _BPJ_MULTIWAVE_H
#define _BPJ_MULTIWAVE_H

#include <byteimage/palette.h>

using byteimage::CachedPalette;

class MultiWaveGenerator {
public:
  class FloatCycle {
  public:
    std::vector<float> values;
    int period = 1;

    float value(int step) const;
  };
  
  class FloatWave {
  public:
    float amplitude = 1.0;
    int period = 1;

    float value(int step) const;
  };

  std::vector<FloatCycle> hue_cycles;
  int hue_period = 1;

  FloatCycle sat_cycle;

  std::vector<FloatWave> lum_waves;

  void load_filename(const char* fn);
  void save_filename(const char* fn) const;
  
  CachedPalette cache(int N) const;
};

#endif
