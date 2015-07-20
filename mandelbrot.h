#ifndef _BPJ_NEWMAN_MANDELBROT_H
#define _BPJ_NEWMAN_MANDELBROT_H

#include "grid.h"
#include "complex.h"

class Mandelbrot {
protected:
  RenderGrid grid;
  std::vector<HPComplex> X, A, B, C;

  void setPrecision();
  
  bool inCardioid(const HPComplex& Z);
  void findProbe();
  void computeOrbit(const HPComplex& X0);
  void computeSeries();
  RenderGrid::EscapeValue getIterations(const HPComplex& Y0);
  RenderGrid::EscapeValue getIterationsHW(const HPComplex& Y0);
  
public:
  int N;
  HPComplex center, sz;

  Mandelbrot();
  Mandelbrot(int nr, int nc);

  //TODO: Remove legacy functionality
  void loadLegacy(const char* fn);
  
  inline int rows() const {return grid.nr;}
  inline int cols() const {return grid.nc;}

  bool useHardware();
  void precompute();
  void computeRow(int r);

  HPComplex pointAt(int r, int c, int sc = 1) const;
  void translate(int dr, int dc, int sc = 1);
  void zoom(float scale);
  void zoomAt(float scale, int r, int c, int sc = 1);
  
  const RenderGrid::EscapeValue& at(int r, int c);
  RenderGrid::EscapeValue at(int r, int c, int sc); //Averages values
  
  void scaleUp(int sc);   //By duplicating values.
  void scaleDown(int sc); //By averaging values.

  void load(const char* fn);
  void save(const char* fn);
};

#endif
