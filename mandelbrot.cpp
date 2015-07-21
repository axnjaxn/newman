#include "mandelbrot.h"
#include <byteimage/types.h>

using namespace byteimage;

Mandelbrot::Mandelbrot() : Mandelbrot(1, 1) { }

Mandelbrot::Mandelbrot(int nr, int nc) : grid(nr, nc) {
  N = 256;

  center.re = -0.5; center.im = 0.0;
  sz.re = 4.0 / nc; sz.im = 3.0 / nr;

  setPrecision();
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

    setPrecision();
  }
}

void Mandelbrot::setPrecision() {
  const double log_alpha = log2(1.0e-20);
  const int beta = 64;

  signed long int e;
  mpf_get_d_2exp(&e, sz.re.get_mpf_t());
  
  int bits = (int)(beta - e + log_alpha);
  if (bits < 64) bits = 64;

  mpf_set_default_prec(bits);//TODO: Determine if this is a necessary line
  center.re.set_prec(bits);
  center.im.set_prec(bits);
  sz.re.set_prec(bits);
  sz.im.set_prec(bits);
  X.clear();
  A.clear();
  B.clear();
  C.clear();
}

constexpr double bailout = 1024.0;
constexpr double bailout2 = bailout * bailout;

inline static bool bailedOut(HPComplex& z) {return sqMag(descend(z)) > bailout2;}

bool Mandelbrot::inCardioid(const HPComplex& Z) {
  mpf_class fourth = 0.25;
  mpf_class xmf = Z.re - fourth;
  mpf_class y2 = Z.im * Z.im;
  mpf_class q = xmf * xmf + y2;
  if (q * (q + xmf) < fourth * y2) return true;//Cardioid
  q = Z.re + 1.0;
  return (q * q + y2 < fourth * fourth);//Second disk
}

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
  X.clear(); X.resize(N);
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
  return (bmag * 1e-5 < cmag);
}

RenderGrid::EscapeValue Mandelbrot::getIterations(const HPComplex& Y0) {
  RenderGrid::EscapeValue escape;
  LPComplex eps, eps2, eps3, a, b, c;
  HPComplex Y;

  if (inCardioid(Y0)) {
    escape.iterations = N;
    escape.smoothing = 0.0;
    return escape;
  }
  
  Y.re = Y0.re - X[0].re;
  Y.im = Y0.im - X[0].im;
  
  eps.re = Y.re.get_d();
  eps.im = Y.im.get_d();
  eps2 = sq(eps);
  eps3 = eps * eps2;

  std::vector<LPComplex> d(X.size());
  d[0] = eps;
  for (int i = 1; i < X.size(); i++) {
    a.re = A[i].re.get_d(); a.im = A[i].im.get_d();
    b.re = B[i].re.get_d(); b.im = B[i].im.get_d();
    c.re = C[i].re.get_d(); c.im = C[i].im.get_d();

    a = a * eps;
    b = b * eps2;
    c = c * eps3;
    if (isUnstable(b, c)) {
      int good = i - 3;
      if (good < 1) good = 1;
      d.resize(good);
      break;
    }
    
    d[i] = a + b + c;
  }

  int found = d.size() - 1;
  Y.re = X[found].re + d[found].re;
  Y.im = X[found].im + d[found].im;
  if (bailedOut(Y)) {  
    int low = 0, high = d.size() - 1, mid = d.size() / 2;
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

    Y.re = X[found].re + d[found].re;
    Y.im = X[found].im + d[found].im;
    escape.iterations = found;
    escape.smoothing = getSmoothingMagnitude(descend(Y));
    return escape;
  }
  
  HPComplex Yn;
  Yn.re = Y.re;
  Yn.im = Y.im;
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

  if (inCardioid(Y0)) {
    escape.iterations = N;
    escape.smoothing = 0.0;
    return escape;
  }
  
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
  //setPrecision();
  X.clear(); A.clear(); B.clear(); C.clear();
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

HPComplex Mandelbrot::pointAt(int r, int c, int sc) const {
  HPComplex pt;
  pt.re = center.re + (sc * c - cols() / 2) * sz.re;
  pt.im = center.im + (rows() / 2 - sc * r - 1) * sz.im;
  return pt;
}

void Mandelbrot::translate(int dr, int dc, int sc) {
  center.re = center.re - dc * sc * sz.re;
  center.im = center.im + dr * sc * sz.im;
}

void Mandelbrot::zoom(float scale) {
  sz.re *= (1.0 / scale);
  sz.im *= (1.0 / scale);
  
  setPrecision();
}

void Mandelbrot::zoomAt(float scale, int r, int c, int sc) {
  HPComplex pt;
  pt.re = center.re + (sc * c - cols() / 2) * sz.re;
  pt.im = center.im + (rows() / 2 - sc * r - 1) * sz.im;
  
  sz.re *= (1.0 / scale);
  sz.im *= (1.0 / scale);
  
  center.re = pt.re - (sc * c - cols() / 2) * sz.re;
  center.im = pt.im - (rows() / 2 - sc * r - 1) * sz.im;

  setPrecision();
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
  RenderGrid scaled(rows() * sc, cols() * sc);

  for (int r = 0; r < scaled.nr; r++)
    for (int c = 0; c < scaled.nc; c++)
      scaled.at(r, c) = at(r / sc, c / sc);

  grid = std::move(scaled);
  sz.re = sz.re / sc;
  sz.im = sz.im / sc;

  setPrecision();
}

void Mandelbrot::scaleDown(int sc) {
  RenderGrid scaled(rows() / sc, cols() / sc);

  for (int r = 0; r < scaled.nr; r++)
    for (int c = 0; c < scaled.nc; c++)
      scaled.at(r, c) = at(r, c, sc);

  grid = std::move(scaled);
  sz.re = sz.re * sc;
  sz.im = sz.im * sc;

  setPrecision();
}

void Mandelbrot::load(const char* fn) { }//TODO: Format TBD

void Mandelbrot::save(const char* fn) { }//TODO: Format TBD
