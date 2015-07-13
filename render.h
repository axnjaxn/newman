#ifndef _BPJ_NEWMANDEL_RENDER_H
#define _BPJ_NEWMANDEL_RENDER_H

#include "grid.h"
#include "complex.h"

class Mandelbrot {
protected:
  std::vector<HPComplex> X, A, B, C;
  
  HPComplex findProbe();
  void computeReference(const HPComplex& X0);
  void computeSeries();
  RenderGrid::EscapeValue getIterations(int r, int c);
  
public:
  int N;
  HPComplex center, sz;
  
  RenderGrid grid;

  Mandelbrot();
  Mandelbrot(int nr, int nc);

  void getCorner(HPComplex& corner) const;
  void setCornerSize(HPComplex& corner, HPComplex& sz);
  void loadLegacy(const char* fn);
  
  inline int rows() const {return grid.nr;}
  inline int cols() const {return grid.nc;}

  bool useHardware();
  void precompute();
  void computeRow(int r);

  RenderGrid::EscapeValue at(int r, int c);
  RenderGrid::EscapeValue at(int r, int c, int sc);

  void load(const char* fn);
  void save(const char* fn);
};

#endif
