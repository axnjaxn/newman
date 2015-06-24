#include <byteimage/byteimage_sdl2.h>
#include <gmpxx.h>

using namespace byteimage;

typedef struct {
  mpf_class re, im;
} HPComplex;

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

double sqMag(const LPComplex& a) {
  return a.re * a.re + a.im * a.im;
}

LPComplex descend(const HPComplex& a) {
  LPComplex b;
  b.re = a.re.get_d();
  b.im = a.im.get_d();
  return b;
}

void getRefOrbit(std::vector<HPComplex>& X,
		 std::vector<HPComplex>& A, std::vector<HPComplex>& B, std::vector<HPComplex>& C,
		 const HPComplex& c) {
  X[0].re = c.re; X[0].im = c.im;
  A[0].re = 1.0;  A[0].im = 0.0;
  B[0].re = 0.0;  B[0].im = 0.0;
  C[0].re = 0.0;  C[0].im = 0.0;

  mpf_class sqmag;
  for (int i = 1; i < X.size(); i++) {
    X[i].re = X[i - 1].re * X[i - 1].re - X[i - 1].im * X[i - 1].im + c.re;
    X[i].im = 2 * X[i - 1].re * X[i - 1].im + c.im;

    if (sqMag(descend(X[i])) > 4.0) {
      X.resize(i);
      A.resize(i);
      B.resize(i);
      C.resize(i);
      return;
    }

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

void blitSampled(ByteImage& target, const ByteImage& src, float sx, float sy, int dr, int dc) {
  if (target.nchannels == 1 && src.nchannels == 3) {
    blitSampled(target, src.toGrayscale(), sx, sy, dr, dc);
    return;
  }
  else if (target.nchannels == 3 && src.nchannels == 1) {
    blitSampled(target, src.toColor(), sx, sy, dr, dc);
    return;
  }
  
  int r0 = dr;
  int c0 = dc;

  int r1 = (int)(dr + sy * (src.nr - 1));
  int c1 = (int)(dc + sx * (src.nc - 1));

  if (r0 < 0) r0 = 0;
  if (c0 < 0) c0 = 0;
  if (r1 >= target.nr) r1 = target.nr - 1;
  if (c1 >= target.nc) c1 = target.nc - 1;

  for (int r = r0, sr, sc; r <= r1; r++)
    for (int c = c0; c <= c1; c++) {
      sr = (int)((r - dr) / sy);
      sc = (int)((c - dc) / sx);
      target.at(r, c, 0) = src.at(sr, sc, 0);
      if (target.nchannels == 3) {
	target.at(r, c, 1) = src.at(sr, sc, 1);
	target.at(r, c, 2) = src.at(sr, sc, 2);
      }
    }      
}

class MyDisplay : public Display {
protected:
  ByteImage canvas, img;
  bool renderflag, redrawflag, previewflag, mousedown;
  HPComplex corner, sz;
  int N = 256, mx, my;
  float scale;

  void handleEvent(SDL_Event event) {
    if (event.type == SDL_KEYDOWN)
      switch (event.key.keysym.sym) {
      case SDLK_F5: redrawflag = true; break;
      case SDLK_BACKSPACE: reset(); break;
      case SDLK_RETURN: previewflag = false; renderflag = true; break;
      case SDLK_SPACE: previewflag = true; renderflag = true; break;
      }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
      mx = event.button.x;
      my = event.button.y;
	
      if (event.button.button == SDL_BUTTON_RIGHT) {
	mousedown = false;
	
	HPComplex pt;
	pt.re = corner.re + mx * sz.re;
	pt.im = corner.im + (img.nr - my - 1) * sz.im;
	corner.re = pt.re - (img.nc / 2) * sz.re;
	corner.im = pt.im - (img.nr / 2) * sz.im;
	
	renderflag = previewflag = true;
      }
      else {
	mousedown = true;
      }
    }
    else if (event.type == SDL_MOUSEMOTION && mousedown) {
      int dy = event.button.y - my;
      scale = (float)pow(16.0, (float)dy / img.nr);
      if (scale > 8.0) scale = 8.0;
      if (scale < 0.125) scale = 0.125;
      redrawflag = true;
    }
    else if (event.type == SDL_MOUSEBUTTONUP && mousedown) {
      mousedown = false;
      
      HPComplex pt;
      pt.re = corner.re + mx * sz.re;
      pt.im = corner.im + (img.nr - my - 1) * sz.im;

      sz.re *= (1.0 / scale);
      sz.im *= (1.0 / scale);
      
      corner.re = pt.re - mx * sz.re;
      corner.im = pt.im - (img.nr - my - 1) * sz.im;

      renderflag = previewflag = true;
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

    redrawflag = true;
  }

  void update() {
    if (renderflag) {
      SDL_SetWindowTitle(window, "Rendering...");
      Uint32 ticks = SDL_GetTicks();
      render();
      ticks = SDL_GetTicks() - ticks;
      char str[256];
      sprintf(str, "Time: %dms", ticks);
      SDL_SetWindowTitle(window, str);
    
      renderflag = false;
      redrawflag = true;
    }

    if (redrawflag) {
      if (!mousedown) {
	canvas = img;
      }
      else
	blitSampled(canvas, img, scale, scale, my - scale * my, mx - scale * mx);
      updateImage(canvas);
      redrawflag = false;
    }
    
    Display::update();
  }

  void reset() {
    previewflag = renderflag = redrawflag = true;

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
