#include <byteimage/byteimage_sdl2.h>
#include <gmpxx.h>

using namespace byteimage;

typedef struct {
  double re, im;
} LPComplex;

LPComplex sq(const LPComplex& a) {
  LPComplex c;
  c.re = a.re * a.re - a.im * a.im;
  c.im = 2 * a.re * a.im;
  return c;
}

LPComplex operator+(const LPComplex& a, const LPComplex& b) {
  LPComplex c;
  c.re = a.re + b.re;
  c.im = a.im + b.im;
  return c;
}

LPComplex operator*(const LPComplex& a, const LPComplex& b) {
  LPComplex c;
  c.re = a.re * b.re - a.im * b.im;
  c.im = a.re * b.im + a.im * b.re;
  return c;
}

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

    A[i].re = 2.0 * (X[i - 1].re * A[i - 1].re - X[i - 1].im * A[i - 1].im) + 1.0;
    A[i].im = 2.0 * (X[i - 1].re * A[i - 1].im + X[i - 1].im * A[i - 1].re);

    B[i].re = 2.0 * (X[i - 1].re * B[i - 1].re - X[i - 1].im * B[i - 1].im) + A[i].re * A[i].re - A[i].im * A[i].im;
    B[i].im = 2.0 * (X[i - 1].re * B[i - 1].im + X[i - 1].im * B[i - 1].re +  A[i].re * A[i].im);

    C[i].re = 2.0 * (X[i - 1].re * C[i - 1].re - X[i - 1].im * C[i - 1].im + A[i].re * B[i].re - A[i].im * B[i].im);
    C[i].im = 2.0 * (X[i - 1].re * C[i - 1].im + X[i - 1].im * C[i - 1].re + A[i].re * B[i].im + A[i].im * B[i].re);
  }
}

bool isUnstable(const LPComplex& bterm, const LPComplex& cterm) {
  double bmag = bterm.re * bterm.re + bterm.im * bterm.im;
  double cmag = cterm.re * cterm.re + cterm.im * cterm.im;
  return (cmag <= 100 * bmag);
}

bool mapflag;

int getIterationsOld(const std::vector<HPComplex>& X, const HPComplex& center, const LPComplex& eps, float bailout) {
  mapflag = true;
  
  bailout *= bailout;
  
  mpf_class dist2;
  LPComplex d, d2, dnext, z;
  HPComplex Y;
  d.re = eps.re;
  d.im = eps.im;
  
  for (int i = 0; i < X.size(); i++) {
    z.re = X[i].re.get_d();
    z.im = X[i].im.get_d();

    d2 = sq(d);
    dnext = z * d;
    dnext.re = 2 * dnext.re + d2.re + eps.re;
    dnext.im = 2 * dnext.im + d2.im + eps.im;

    Y.re = X[i].re + dnext.re;
    Y.im = X[i].im + dnext.im;

    dist2 = Y.re * Y.re + Y.im * Y.im;
    if (dist2.get_d() > bailout) return i;

    d = dnext;
  }
  
  return X.size();
}

int getIterations(const std::vector<HPComplex>& X,
		  const std::vector<HPComplex>& A, const std::vector<HPComplex>& B, const std::vector<HPComplex>& C,
		  const HPComplex& center, const HPComplex& pt, float bailout) {
  bailout *= bailout;
  
  mpf_class dist2;
  LPComplex eps, eps2, eps3, a, b, c, d;
  HPComplex Y;
  Y.re = pt.re - center.re;
  Y.im = pt.im - center.im;
  
  eps.re = Y.re.get_d();
  eps.im = Y.im.get_d();
  eps2 = sq(eps);
  eps3 = eps * eps2;
  
  for (int i = 0; i < X.size(); i++) {
    a.re = A[i].re.get_d(); a.im = A[i].im.get_d();
    b.re = B[i].re.get_d(); b.im = B[i].im.get_d();
    c.re = C[i].re.get_d(); c.im = C[i].im.get_d();

    a = a * eps;
    b = b * eps2;
    c = c * eps3;
    d = a + b + c;

    if (isUnstable(b, c)) return getIterationsOld(X, center, eps, bailout);

    Y.re = X[i].re + d.re;
    Y.im = X[i].im + d.im;

    dist2 = Y.re * Y.re + Y.im * Y.im;
    if (dist2.get_d() > bailout) return i;
  }
  
  return X.size();
}

class MyDisplay : public Display {
protected:
  ByteImage canvas, img;
  bool renderflag, drawflag, previewflag;
  HPComplex corner, sz;
  int N = 256;

  void handleEvent(SDL_Event event) {
    if (event.type == SDL_KEYDOWN)
      switch (event.key.keysym.sym) {
      case SDLK_F5: drawflag = true; break;
      case SDLK_BACKSPACE: reset(); break;
      case SDLK_RETURN: previewflag = false; renderflag = true; break;
      case SDLK_SPACE: previewflag = true; renderflag = true; break;
      }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_RIGHT) {
	int x = event.button.x;
	int y = event.button.y;
	
	HPComplex pt;
	pt.re = corner.re + x * sz.re;
	pt.im = corner.im + (img.nr - y - 1) * sz.im;
	
	sz.re *= 0.5;
	sz.im *= 0.5;
	
	corner.re = pt.re - (img.nc / 2) * sz.re;
	corner.im = pt.im - (img.nr / 2) * sz.im;
	
	renderflag = previewflag = true;
      }
      else {
	//TODO
      }
    }

    Display::handleEvent(event);
  }

  void preview() {
    HPComplex pt;
    int its;
    LPComplex Z, C;
    for (int r = 0; r < img.nr; r++)
      for (int c = 0; c < img.nc; c++) {
	pt.re = corner.re + c * sz.re;
	pt.im = corner.im + (img.nr - r - 1) * sz.im;
	Z.re = C.re = pt.re.get_d();
	Z.im = C.im = pt.im.get_d();
	for (its = 0; its < N; its++) {
	  Z = sq(Z) + C;
	  if (Z.re * Z.re + Z.im * Z.im > 4.0) break;
	}
	img.at(r, c, 0) = img.at(r, c, 1) = img.at(r, c, 2) = clip(255 - its);
      }
  }
  
  void render() {
    if (previewflag) {preview(); return;}
    
    std::vector<HPComplex> X(N), A(N), B(N), C(N);
    
    Uint32 ticks = SDL_GetTicks();
  
    HPComplex center;
    center.re = corner.re + (img.nc / 2) * sz.re;
    center.im = corner.im + (img.nr / 2) * sz.im;
    getRefOrbit(X, A, B, C, center);
  
    HPComplex pt;
    int its;
    for (int r = 0; r < img.nr; r++)
      for (int c = 0; c < img.nc; c++) {
	mapflag = false;
	pt.re = corner.re + c * sz.re;
	pt.im = corner.im + (img.nr - r - 1) * sz.im;
	its = getIterations(X, A, B, C, center, pt, 2.0);
	img.at(r, c, 0) = img.at(r, c, 1) = img.at(r, c, 2) = clip(255 - its);
	if (mapflag) img.at(r, c, 1) = 255;
      }

    ticks = SDL_GetTicks() - ticks;
    printf("Time: %dms\n", ticks);

    drawflag = true;
  }

  void update() {
    if (renderflag) {
      render();
      renderflag = false;
      drawflag = true;
    }

    if (drawflag) {
      canvas = img;
      updateImage(canvas);
      drawflag = false;
    }
    
    Display::update();
  }

  void reset() {
    previewflag = renderflag = drawflag = true;

    corner.re = -2.5;
    corner.im = -1.5;
    
    sz.re = (1.5 - corner.re) / img.nc;
    sz.im = (1.5 - corner.im) / img.nr;
  }
  
public:
  MyDisplay(int h, int w) : Display(h, w), img(h, w, 3) {reset();}
};

int main(int argc, char* argv[]) {
  const int w = 800;
  const int h = 600;
  
  MyDisplay(h, w).main();
  
  return 0;
}
