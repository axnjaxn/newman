#ifndef _BPJ_NEWMANDEL_GRID_H
#define _BPJ_NEWMANDEL_GRID_H

#include <vector>

class RenderGrid {
public:
  class EscapeValue {
  public:
    int iterations;
    float smoothing;

    EscapeValue(int iterations = 0, float smoothing = 0.0)
      : iterations(iterations), smoothing(smoothing) { }
  };

  int nr, nc;
  std::vector<EscapeValue> values;

  RenderGrid() : nr(0), nc(0) { }
  RenderGrid(int nr, int nc) : nr(nr), nc(nc), values(nr * nc) { }

  EscapeValue& at(int r, int c) {return values[r * nc + c];}
  const EscapeValue& at(int r, int c) const {return values[r * nc + c];}
};

#endif
