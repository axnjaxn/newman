#include <byteimage/byteimage_sdl2.h>
#include <byteimage/palette.h>
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
  return (cmag >= 100 * bmag);
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
  bool renderflag, redrawflag, previewflag, drawlines;
  HPComplex corner, sz;
  int N;
  int mousedown, mx, my, nx, ny;
  float scale;
  LinearPalette pal; int palN;

  void handleEvent(SDL_Event event) {
    if (event.type == SDL_KEYDOWN)
      switch (event.key.keysym.sym) {
      case SDLK_F5: redrawflag = true; break;
      case SDLK_BACKSPACE: reset(); break;
      case SDLK_RETURN: previewflag = false; renderflag = true; break;
      case SDLK_SPACE: previewflag = true; renderflag = true; break;
      case SDLK_UP: N += 256; printf("%d iterations\n", N); previewflag = true; renderflag = true; break;
      case SDLK_DOWN: if (N > 256) N -= 256; printf("%d iterations\n", N); previewflag = true; renderflag = true; break;
      case SDLK_F2: save(); break;
      case SDLK_F3: load(); break;
      case SDLK_d:
	drawlines = !drawlines;
	printf("Draw lines mode: %s\n", drawlines? "on" : "off");
	break;
      case SDLK_i:
	printf("How many iterations?\n");
	scanf("%d", &N);
	printf("%d iterations.\n", N);
	previewflag = renderflag = true;
	break;
      }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
      mx = event.button.x;
      my = event.button.y;
      scale = 1.0;
	
      if (event.button.button == SDL_BUTTON_RIGHT) mousedown = 2;
      else mousedown = 1;
    }
    else if (event.type == SDL_MOUSEMOTION) {
      if (mousedown == 1) {
	int dx = event.button.x - mx;
	int dy = event.button.y - my;
	float dist = sqrt(dx * dx + dy * dy);
	if (SDL_GetModState() & KMOD_SHIFT) dist = -dist;
	scale = (float)pow(32.0, dist / img.nr);
	redrawflag = true;
      }
      else if (mousedown == 2) {
	nx = event.button.x;
	ny = event.button.y;
	redrawflag = true;
      }
    }
    else if (event.type == SDL_MOUSEBUTTONUP && mousedown) {
      if (mousedown == 1) {
	HPComplex pt;
	pt.re = corner.re + mx * sz.re;
	pt.im = corner.im + (img.nr - my - 1) * sz.im;
	
	sz.re *= (1.0 / scale);
	sz.im *= (1.0 / scale);
	
	corner.re = pt.re - mx * sz.re;
	corner.im = pt.im - (img.nr - my - 1) * sz.im;
	
	renderflag = previewflag = true;
      }

      else if (mousedown == 2) {
	corner.re = corner.re + (mx - nx) * sz.re;
	corner.im = corner.im + (ny - my) * sz.im;

	renderflag = previewflag = true;
      }

      mousedown = 0;
    }

    Display::handleEvent(event);
  }

  Color getColor(int its) {
    if (its == N) return Color(0);
    return pal.inUnit((float)its / palN);    
  }

  bool drawLine() {
    updateImage(img);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) {
	SDL_PushEvent(&event);
	return true;
      }
      Display::handleEvent(event);
    }
    Display::update();
    if (exitflag) return true;
  }
  
  void preview() {
    HPComplex pt;
    int its;
    LPComplex Z, Z0;
    Color color;
    Byte *Rp = img.R(), *Gp = img.G(), *Bp = img.B();
    if (drawlines) img = canvas;
    for (int r = 0, i = 0; r < img.nr; r++) {
      for (int c = 0; c < img.nc; c++, i++) {
	pt.re = corner.re + c * sz.re;
	pt.im = corner.im + (img.nr - r - 1) * sz.im;
	Z.re = Z0.re = pt.re.get_d();
	Z.im = Z0.im = pt.im.get_d();
	for (its = 0; its < N; its++) {
	  Z = sq(Z) + Z0;
	  if (Z.re * Z.re + Z.im * Z.im > 4.0) break;
	}
	color = getColor(its);
	Rp[i] = color.r; Gp[i] = color.g; Bp[i] = color.b;
      }
      if (drawlines && drawLine()) break;
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
    Color color;
    Byte *Rp = img.R(), *Gp = img.G(), *Bp = img.B();
    if (drawlines) img = canvas;
    for (int r = 0, i = 0; r < img.nr; r++) {
      for (int c = 0; c < img.nc; c++, i++) {
	mapflag = false;
	pt.re = corner.re + c * sz.re;
	pt.im = corner.im + (img.nr - r - 1) * sz.im;
	its = getIterations(X, A, B, C, center, pt, 2.0);
	color = getColor(its);
	Rp[i] = color.r; Gp[i] = color.g; Bp[i] = color.b;
	//if (mapflag) img.at(r, c, 1) = 255;
      }
      if (drawlines && drawLine()) break;
    }

    redrawflag = true;
  }

  void update() {
    frameDelay = 0;
    
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
      if (mousedown == 1){
	canvas.fill(255);
	blitSampled(canvas, img, scale, scale, my - scale * my, mx - scale * mx);
      }
      else if (mousedown == 2) {
	canvas.fill(255);
	canvas.blit(img, ny - my, nx - mx);
      }
      else {
	canvas = img;
      }
      updateImage(canvas);
      redrawflag = false;
    }

    if (!mousedown) frameDelay = 100;
    
    Display::update();
  }

  void reset() {
    previewflag = renderflag = redrawflag = drawlines = true;

    N = 256;
    
    corner.re = -2.5;
    corner.im = -1.5;
    
    sz.re = (1.5 - corner.re) / img.nc;
    sz.im = (1.5 - corner.im) / img.nr;
  }

  void constructDefaultPalette() {
    CachedPalette src = CachedPalette::fromColors({Color(255), Color(255, 64, 0), Color(0, 64, 255), Color(128, 255, 64), Color(255, 255, 0), Color(0, 0, 255),
	  Color(128, 128, 192), Color(255, 0, 255), Color(255, 192, 192), Color(128, 160, 64),
	  Color(192, 64, 0), Color(32, 32, 160), Color(160, 192, 32), Color(255, 32, 192), Color(64, 0, 192),
	  Color(32, 128, 192), Color(32, 0, 255), Color(160, 32, 96), Color(255, 160, 64)});
    pal = LinearPalette(src);
    palN = (src.levels() - 1) * 256;
    
  }

  void save() {
    HPComplex center;
    center.re = corner.re + (img.nc / 2) * sz.re;
    center.im = corner.im + (img.nr / 2) * sz.im;

    printf("Enter a filename:\n");
    char fn[256];
    scanf("%s", fn);

    FILE* fp = fopen(fn, "w");
    if (fp) {
      fprintf(fp, "%d\n", N);
      mpf_out_str(fp, 10, 0, center.re.get_mpf_t());
      fprintf(fp, "\n");
      mpf_out_str(fp, 10, 0, center.im.get_mpf_t());
      fprintf(fp, "\n");
      mpf_out_str(fp, 10, 0, sz.re.get_mpf_t());
      fprintf(fp, "\n");
      mpf_out_str(fp, 10, 0, sz.im.get_mpf_t());
      fprintf(fp, "\n");
      fclose(fp);
      printf("Saved to %s\n", fn);
    }
    else {
      printf("Could not save to %s\n", fn);
    }    
  }

  void load() {
    printf("Enter a filename:\n");
    char fn[256];
    scanf("%s", fn);

    char buf[10 * 1024];

    HPComplex center;
    FILE* fp = fopen(fn, "r");
    if (fp) {
      fscanf(fp, "%d\n", &N);
      fscanf(fp, "%s\n", buf); center.re = buf;
      fscanf(fp, "%s\n", buf); center.im = buf;
      fscanf(fp, "%s\n", buf); sz.re = buf;
      fscanf(fp, "%s\n", buf); sz.im = buf;
      fclose(fp);

      corner.re = center.re - (img.nc / 2) * sz.re;
      corner.im = center.im - (img.nr / 2) * sz.im;
      
      previewflag = renderflag = true;
      printf("Loaded from %s\n", fn);
    }
    else {
      printf("Could not load from %s\n", fn);
    }
  }
  
public:
  MyDisplay(int h, int w) : Display(h, w), img(h, w, 3), canvas(h, w) {
    for (int r = 0; r < h; r++)
      for (int c = 0; c < w; c++)
	canvas.at(r, c) = (((r / 4) + (c / 4)) & 1)? 255 : 192;
    canvas = canvas.toColor();
	  
    constructDefaultPalette();
    reset();
  }
};

int main(int argc, char* argv[]) {
  const int w = 800;
  const int h = 600;
  
  MyDisplay(h, w).main();
  
  return 0;
}
