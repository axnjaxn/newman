#include <byteimage/byteimage_sdl2.h>
#include <gmpxx.h>

using namespace byteimage;

typedef struct {
  double re, im;
} LPComplex;

typedef struct {
  mpf_class re, im;
} HPComplex;

void getRefOrbit(std::vector<HPComplex>& X,
		 std::vector<HPComplex>& A, std::vector<HPComplex>& B, std::vector<HPComplex>& C,
		 const HPComplex& c) {
  X[0].re = c.re; X[0].im = c.im;
  A[0].re = 1.0;  A[0].im = 0.0;
  B[0].re = 0.0;  B[0].im = 0.0;
  C[0].re = 0.0;  C[0].im = 0.0;
  
  for (int i = 1; i < X.size(); i++) {
    X[i].re = X[i - 1].re * X[i - 1].re - X[i - 1].im * X[i - 1].im + c.re;
    X[i].im = 2 * X[i - 1].re * X[i - 1].im + c.im;

    A[i].re = 2.0 * (X[i - 1].re * A[i - 1].re - X[i - 1].im * A[i - 1].im) + 1;
    A[i].im = 2.0 * (X[i - 1].re * A[i - 1].im + X[i - 1].im * A[i - 1].re);

    B[i].re = 2.0 * (X[i - 1].re * B[i - 1].re - X[i - 1].im * B[i - 1].im) + A[i].re * A[i].re - A[i].im * A[i].im;
    B[i].im = 2.0 * (X[i - 1].re * B[i - 1].im + X[i - 1].im * B[i - 1].re + A[i].re * A[i].im);

    C[i].re = 2.0 * (X[i - 1].re * C[i - 1].re - X[i - 1].im * C[i - 1].im + A[i].re * B[i].re - A[i].im * B[i].im);
    C[i].im = 2.0 * (X[i - 1].re * C[i - 1].im + X[i - 1].im * C[i - 1].re + A[i].re * B[i].im + A[i].im * B[i].re);
  }
}

int getIterations(const std::vector<HPComplex>& orbit, const HPComplex& center,
		  const HPComplex& c, float bailout) {
  bailout *= bailout;
  
  mpf_class dist2;
  LPComplex eps, d, d2, dnext, z;
  HPComplex Y;
  Y.re = c.re - center.re;
  Y.im = c.im - center.im;
  eps.re = d.re = Y.re.get_d();
  eps.im = d.im = Y.im.get_d();
  
  for (int i = 0; i < orbit.size(); i++) {
    z.re = orbit[i].re.get_d();
    z.im = orbit[i].im.get_d();
    
    d2.re = d.re * d.re - d.im * d.im;
    d2.im = 2 * d.re * d.im;

    dnext.re = 2 * (z.re * d.re - z.im * d.im) + d2.re + eps.re;
    dnext.im = 2 * (z.re * d.im + z.im * d.re) + d2.im + eps.im;

    Y.re = orbit[i].re + dnext.re;
    Y.im = orbit[i].im + dnext.im;

    dist2 = Y.re * Y.re + Y.im * Y.im;
    if (dist2.get_d() > bailout) return i;
    
    d.re = dnext.re;
    d.im = dnext.im;
  }
  
  return orbit.size();
}

int main(int argc, char* argv[]) {
  const int w = 800;
  const int h = 600;
  
  Display display(h, w);
  Uint32 ticks = SDL_GetTicks();
  
  ByteImage img(h, w);

  HPComplex corner;
  corner.re = -2.5;
  corner.im = -1.5;

  mpf_class sz = 0.005;

  std::vector<HPComplex> X(256), A(256), B(256), C(256);
  HPComplex center;
  center.re = corner.re + (w / 2) * sz;
  center.im = corner.im + (h / 2) * sz;
  getRefOrbit(X, A, B, C, center);
  
  HPComplex pt;
  int its;
  for (int r = 0; r < img.nr; r++)
    for (int c = 0; c < img.nc; c++) {
      pt.re = corner.re + c * sz;
      pt.im = corner.im + (img.nr - r - 1) * sz;
      its = getIterations(X, center, pt, 2.0);
      img.at(r, c) = 255 - its;
    }

  ticks = SDL_GetTicks() - ticks;
  printf("Time: %dms\n", ticks);

  if (!display.show(img)) 
    display.main();
  
  return 0;
}
