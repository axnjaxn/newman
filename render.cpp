#include "render.h"

Mandelbrot::Mandelbrot() : Mandelbrot(1, 1) { }

Mandelbrot::Mandelbrot(int nr, int nc) : grid(nr, nc) {
  N = 256;

  center.re = -1.0; center.im = 0.0;
  sz.re = 4.0 / nc; sz.im = 3.0 / nr;
}

void Mandelbrot::getCorner(HPComplex& corner) const {
  corner.re = center.re - (cols() / 2) * sz.re;
  corner.im = center.re - (rows() / 2) * sz.re;  
}

void Mandelbrot::setCornerSize(HPComplex& corner, HPComplex& sz) {
  center.re = corner.re + (cols() / 2) * sz.re;
  center.im = corner.re + (rows() / 2) * sz.re;
  this->sz = sz;
}

void Mandelbrot::loadLegacy(const char* fn) {
  FILE* fp = fopen(fn, "r");
  if (fp) {
    char buf[1024];
    fscanf(fp, "%d\n", &N);
    fscanf(fp, "%s\n", buf); center.re = buf;
    fscanf(fp, "%s\n", buf); center.im = buf;
    fscanf(fp, "%s\n", buf); sz.re = buf;
    fscanf(fp, "%s\n", buf); sz.im = buf;
    fclose(fp);

    sz.re = sz.re * (800.0 / cols());
    sz.im = sz.im * (600.0 / rows());
  }
}
