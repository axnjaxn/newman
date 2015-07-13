#include "render.h"
#include <byteimage/types.h>

using namespace byteimage;

Mandelbrot::Mandelbrot() : Mandelbrot(1, 1) { }

Mandelbrot::Mandelbrot(int nr, int nc) : grid(nr, nc) {
  N = 256;

  center.re = -0.5; center.im = 0.0;
  sz.re = 4.0 / nc; sz.im = 3.0 / nr;  
}

void Mandelbrot::getCorner(HPComplex& corner) const {
  corner.re = center.re - (cols() / 2) * sz.re;
  corner.im = center.im - (rows() / 2) * sz.im;  
}

void Mandelbrot::setCornerSize(HPComplex& corner, HPComplex& sz) {
  center.re = corner.re + (cols() / 2) * sz.re;
  center.im = corner.im + (rows() / 2) * sz.im;
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

constexpr double bailout = 1024.0;
constexpr double bailout2 = bailout * bailout;

inline static bool bailedOut(HPComplex& z) {return sqMag(descend(z)) > bailout2;}

void Mandelbrot::findProbe() {
  std::vector<Pt> probe_pts;
  HPComplex probe;

  for (int c = 0; c < cols(); c += 2) {
    probe_pts.push_back(Pt(rows() / 4, c));
    probe_pts.push_back(Pt(rows() / 2, c));
    probe_pts.push_back(Pt(3 * rows() / 4, c));
  }
  for (int r = 0; r < rows(); r += 2)
    probe_pts.push_back(Pt(r, cols() / 2));
  
  for (auto pt : probe_pts) {
    probe.re = center.re + (pt.c - cols() / 2) * sz.re;
    probe.im = center.im + (rows() / 2 - pt.r - 1) * sz.im;
    computeOrbit(probe);
    
    if (X.size() > A.size())
      std::swap(X, A);
  }

  std::swap(X, A);
}

void Mandelbrot::computeOrbit(const HPComplex& X0) {
  X.resize(N);
  X[0].re = X0.re; X[0].im = X0.im;

  for (int i = 1; i < X.size(); i++) {
    X[i].re = X[i - 1].re * X[i - 1].re - X[i - 1].im * X[i - 1].im + X0.re;
    X[i].im = 2.0 * (X[i - 1].re * X[i - 1].im) + X0.im;
    
    if (bailedOut(X[i])) {
      X.resize(i);
      return;
    }
  }
}

void Mandelbrot::computeSeries() {
  A.resize(X.size());
  B.resize(X.size());
  C.resize(X.size());
  
  A[0].re = 1.0;  A[0].im = 0.0;
  B[0].re = 0.0;  B[0].im = 0.0;
  C[0].re = 0.0;  C[0].im = 0.0;

  for (int i = 1; i < X.size(); i++) {
    A[i].re = 2.0 * (X[i - 1].re * A[i - 1].re - X[i - 1].im * A[i - 1].im) + 1.0;
    A[i].im = 2.0 * (X[i - 1].re * A[i - 1].im + X[i - 1].im * A[i - 1].re);

    B[i].re = 2.0 * (X[i - 1].re * B[i - 1].re - X[i - 1].im * B[i - 1].im) + A[i].re * A[i].re - A[i].im * A[i].im;
    B[i].im = 2.0 * (X[i - 1].re * B[i - 1].im + X[i - 1].im * B[i - 1].re + A[i].re * A[i].im);

    C[i].re = 2.0 * (X[i - 1].re * C[i - 1].re - X[i - 1].im * C[i - 1].im + A[i].re * B[i].re - A[i].im * B[i].im);
    C[i].im = 2.0 * (X[i - 1].re * C[i - 1].im + X[i - 1].im * C[i - 1].re + A[i].re * B[i].im + A[i].im * B[i].re);
  }
}

static double getSmoothingMagnitude(const LPComplex& z) {
  double r2 = sqMag(z);
  return 1.0 - log2(0.5 * log(r2) / log(bailout));
}

static bool isUnstable(const LPComplex& bterm, const LPComplex& cterm) {
  double bmag = bterm.re * bterm.re + bterm.im * bterm.im;
  double cmag = cterm.re * cterm.re + cterm.im * cterm.im;
  return (cmag && 1e5 * cmag >= bmag);
}

RenderGrid::EscapeValue Mandelbrot::getIterations(const HPComplex& Y0) {
  RenderGrid::EscapeValue escape;
  LPComplex eps, eps2, eps3, a, b, c;
  HPComplex Y;
  Y.re = Y0.re - X[0].re;
  Y.im = Y0.im - X[0].im;
  
  eps.re = Y.re.get_d();
  eps.im = Y.im.get_d();
  eps2 = sq(eps);
  eps3 = eps * eps2;

  std::vector<LPComplex> d(X.size());
  for (int i = 0; i < X.size(); i++) {
    a.re = A[i].re.get_d(); a.im = A[i].im.get_d();
    b.re = B[i].re.get_d(); b.im = B[i].im.get_d();
    c.re = C[i].re.get_d(); c.im = C[i].im.get_d();

    a = a * eps;
    b = b * eps2;
    c = c * eps3;
    if (isUnstable(b, c)) {
      d.resize(i / 2 + 1);
      break;
    }
    
    d[i] = a + b + c;
  }

  int low = 0, high = d.size() - 1, mid = d.size() / 2, found = d.size();
  while (low <= high) {
    Y.re = X[mid].re + d[mid].re;
    Y.im = X[mid].im + d[mid].im;

    if (!bailedOut(Y))
      low = mid + 1;
    else {
      high = mid - 1;
      found = mid;
    }
    
    mid = (low + high) / 2;
  }
  if (found < d.size()) {
    escape.iterations = found;
    escape.smoothing = getSmoothingMagnitude(d[found]);
    return escape;
  }
  
  HPComplex Yn;
  Y.re = Yn.re = X[d.size() - 1].re + d[d.size() - 1].re;
  Y.im = Yn.im = X[d.size() - 1].im + d[d.size() - 1].im;
  for (int i = d.size(); i < N; i++) {
    Yn.re = Y.re * Y.re - Y.im * Y.im + Y0.re;
    Yn.im = 2.0 * (Y.re * Y.im) + Y0.im;

    if (bailedOut(Yn)) {
      escape.iterations = i;
      escape.smoothing = getSmoothingMagnitude(descend(Yn));
      return escape;
    }

    Y.re = Yn.re;
    Y.im = Yn.im;      
  }

  escape.iterations = N;
  escape.smoothing = 0.0;
  return escape;
}

RenderGrid::EscapeValue Mandelbrot::getIterationsHW(const HPComplex& Y0) {
  RenderGrid::EscapeValue escape;
  HPComplex pt;
  LPComplex Z, Z0 = descend(Y0);

  Z = Z0;
  for (escape.iterations = 0; escape.iterations < N; escape.iterations++) {
    Z = sq(Z) + Z0;
    if (sqMag(Z) > bailout2) break;
  }
  
  if (escape.iterations < N)
    escape.smoothing = getSmoothingMagnitude(Z);
  else
    escape.smoothing = 0.0;

  return escape;
}

bool Mandelbrot::useHardware() {
  const double minpreview = 1.5e-16;
  return (sz.re.get_d() >= minpreview && sz.im.get_d() >= minpreview);
}

void Mandelbrot::precompute() {
  if (useHardware()) return;
  findProbe();
  computeSeries();
}

void Mandelbrot::computeRow(int r) {
  HPComplex pt;
  pt.im = center.im + (rows() / 2 - r - 1) * sz.im;

  if (useHardware())
    for (int c = 0; c < cols(); c++) {
      pt.re = center.re + (c - cols() / 2) * sz.re;
      grid.at(r, c) = getIterationsHW(pt);
    }
  else
    for (int c = 0; c < cols(); c++) {
      pt.re = center.re + (c - cols() / 2) * sz.re;
      grid.at(r, c) = getIterations(pt);
    }
}

const RenderGrid::EscapeValue& Mandelbrot::at(int r, int c) {return grid.at(r, c);}

RenderGrid::EscapeValue Mandelbrot::at(int r, int c, int sc) {
  RenderGrid::EscapeValue escape;
  float sum = 0.0;
  for (int r1 = 0; r1 < sc; r1++)
    for (int c1 = 0; c1 < sc; c1++) {
      escape = at(sc * r + r1, sc * c + c1);
      sum += escape.iterations + escape.smoothing;      
    }
  sum /= sc * sc;
  escape.iterations = (int)sum;
  escape.smoothing = sum - escape.iterations;
  return escape;
}

void Mandelbrot::scaleUp(int sc) {
  Mandelbrot scaled(rows() * sc, cols() * sc);

  for (int r = 0; r < scaled.rows(); r++)
    for (int c = 0; c < scaled.cols(); c++)
      scaled.grid.at(r, c) = at(r / sc, c / sc);
  
  *this = std::move(scaled);
}

void Mandelbrot::scaleDown(int sc) {
  Mandelbrot scaled(rows() / sc, cols() / sc);

  for (int r = 0; r < scaled.rows(); r++)
    for (int c = 0; c < scaled.cols(); c++)
      scaled.grid.at(r, c) = at(r, c, sc);
  
  *this = std::move(scaled);
}

void Mandelbrot::load(const char* fn) { }//TODO: Format TBD

void Mandelbrot::save(const char* fn) { }//TODO: Format TBD
