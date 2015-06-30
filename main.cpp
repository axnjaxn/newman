#include <byteimage/osd.h>
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

constexpr double bailout = 1024.0;
constexpr double bailout2 = bailout * bailout;

inline bool bailedOut(HPComplex& z) {
  return sqMag(descend(z)) > bailout2;
}

double getSmoothingMagnitude(const LPComplex& z) {
  double r2 = sqMag(z);
  return 1.0 - log2(0.5 * log(r2) / log(bailout));
}

void getRefOrbit(std::vector<HPComplex>& X, const HPComplex& X0) {
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

void getSeries(std::vector<HPComplex>& A, std::vector<HPComplex>& B, std::vector<HPComplex>& C,
	       const std::vector<HPComplex>& X) {
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

bool isUnstable(const LPComplex& bterm, const LPComplex& cterm) {
  double bmag = bterm.re * bterm.re + bterm.im * bterm.im;
  double cmag = cterm.re * cterm.re + cterm.im * cterm.im;
  return (cmag && 1e5 * cmag >= bmag);
}

int getIterations(const std::vector<HPComplex>& X,
		  const std::vector<HPComplex>& A, const std::vector<HPComplex>& B, const std::vector<HPComplex>& C,
		  const HPComplex& ref, const HPComplex& Y0, int N,
		  double& smoothing) {
  LPComplex eps, eps2, eps3, a, b, c;
  HPComplex Y;
  Y.re = Y0.re - ref.re;
  Y.im = Y0.im - ref.im;
  
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
    smoothing = getSmoothingMagnitude(d[found]);
    return found;
  }
  
  HPComplex Yn;
  Y.re = Yn.re = X[d.size() - 1].re + d[d.size() - 1].re;
  Y.im = Yn.im = X[d.size() - 1].im + d[d.size() - 1].im;
  for (int i = d.size(); i < N; i++) {
    Yn.re = Y.re * Y.re - Y.im * Y.im + Y0.re;
    Yn.im = 2.0 * (Y.re * Y.im) + Y0.im;

    if (bailedOut(Yn)) {
      smoothing = getSmoothingMagnitude(descend(Yn));
      return i;
    }

    Y.re = Yn.re;
    Y.im = Yn.im;      
  }
  
  return N;
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

float interpHue(float f1, float f2, float t) {
  float diff = f2 - f1;
  if (diff > 180.0) diff = diff - 360.0;
  else if (diff <= -180.0) diff = diff + 360.0;
  f2 = f1 + t * diff;
  if (f2 < 0.0) return f2 + 360.0;
  else if (f2 >= 360.0) return f2 - 360.0;
  else return f2;
}

CachedPalette getMultiWave(int N) {
  FILE* fp = fopen("palette.pal", "r");
  
  int nhuecycles;
  fscanf(fp, "%d", &nhuecycles);

  std::vector< std::vector<float> > hue_cycles;
  std::vector<int> periods(nhuecycles);
  for (int i = 0; i < nhuecycles; i++) {
    int nhues;
    fscanf(fp, "%d", &nhues);

    std::vector<float> cycle(nhues);
    for (int j = 0; j < cycle.size(); j++)
      fscanf(fp, "%f", &cycle[j]);
    hue_cycles.push_back(cycle);
    
    fscanf(fp, "%d", &periods[i]);
  }

  int long_period;
  fscanf(fp, "%d", &long_period);

  int nsats;
  fscanf(fp, "%d", &nsats);
  std::vector<float> sats(nsats);
  for (int i = 0; i < sats.size(); i++)
    fscanf(fp, "%f", &sats[i]);

  int sat_period;
  fscanf(fp, "%d", &sat_period);

  int nlums;
  fscanf(fp, "%d", &nlums);
  std::vector<float> lums(nlums);
  std::vector<int> lum_periods(nlums);
  for (int i = 0; i < lums.size(); i++) {
    fscanf(fp, "%f", &lums[i]);
    fscanf(fp, "%d", &lum_periods[i]);
  }
    
  fclose(fp);

  CachedPalette pal(N);

  for (int i = 0; i < N; i++) {
    float th0 = hue_cycles.size() * (i % long_period) / (float)long_period;
    int c0 = (int)th0;
    int c1 = (c0 + 1) % hue_cycles.size();

    float t0 = hue_cycles[c0].size() * (i % periods[c0]) / (float)periods[c0];
    int x0 = (int)t0;
    int x1 = (x0 + 1) % hue_cycles[c0].size();
    t0 = interpHue(hue_cycles[c0][x0], hue_cycles[c0][x1], t0 - x0);
    
    float t1 = hue_cycles[c1].size() * (i % periods[c1]) / (float)periods[c1];
    int y0 = (int)t1;
    int y1 = (y0 + 1) % hue_cycles[c1].size();
    t1 = interpHue(hue_cycles[c1][y0], hue_cycles[c1][y1], t1 - y0);

    th0 = interpHue(t0, t1, th0 - c0);

    float ts = sats.size() * (i % sat_period) / (float)sat_period;
    int s0 = (int)ts;
    int s1 = (s0 + 1) % sats.size();
    ts -= s0;
    ts = (1.0 - ts) * sats[s0] + ts * sats[s1];

    float tl = 0.0;
    for (int j = 0; j < lums.size(); j++)
      tl += lums[j] * sin(i * 2.0 * 3.14159265358979 / lum_periods[j]);
    tl = 1.0 / (1.0 + exp(-tl));
    
    hsl2rgb(th0, ts, tl, pal[i].r, pal[i].g, pal[i].b);
  }
  
  return pal;
}

class MyDisplay : public Display {
protected:
  ByteImage canvas, img;
  OSD_Printer osd;
  OSD_Scanner scanner;
  bool renderflag, redrawflag, drawlines, smoothflag;
  HPComplex corner, sz;
  int N;
  int mousedown, mx, my, nx, ny;
  float scale;
  CachedPalette pal;

  void screenshot() {
    char fn[256];
    int t = (int)time(NULL);
    sprintf(fn, "%d.png", t);
    img.save_filename(fn);
    osd.print(OSD_Printer::string("Saved screenshot to %s", fn));
  }
  
  void handleEvent(SDL_Event event) {
    int n;
    if (event.type == SDL_KEYDOWN)
      switch (event.key.keysym.sym) {
      case SDLK_F5: redrawflag = true; break;
      case SDLK_BACKSPACE: reset(); break;
      case SDLK_RETURN: renderflag = true; break;
      case SDLK_UP:
	N += 256;
	renderflag = true;
	osd.print(OSD_Printer::string("%d iterations", N));
	break;
      case SDLK_DOWN:
	if (N > 256) N -= 256;
	renderflag = true;
	osd.print(OSD_Printer::string("%d iterations", N));
	break;
      case SDLK_F2: save(); break;
      case SDLK_F3: constructDefaultPalette(); load(); break;
      case SDLK_p: constructNewPalette(); renderflag = true; break;
      case SDLK_F11: screenshot(); break;
      case SDLK_s:
	smoothflag = !smoothflag;
	osd.print(OSD_Printer::string("Smoothing: %s", smoothflag? "on" : "off"));
	renderflag = true;
	break;
      case SDLK_d:
	drawlines = !drawlines;
	osd.print(OSD_Printer::string("Draw lines mode: %s", drawlines? "on" : "off"));
	break;
      case SDLK_i:
	if (scanner.getInt(canvas, "How many iterations?", n)) {
	  N = n;
	  osd.print(OSD_Printer::string("%d iterations.", N));
	  renderflag = true;
	}
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
	
	renderflag = true;
      }

      else if (mousedown == 2) {
	HPComplex mv;
	mv.re = mx - nx;
	mv.im = ny - my;
	corner.re = corner.re + mv.re * sz.re;
	corner.im = corner.im + mv.im * sz.im;

	renderflag = true;
      }

      mousedown = 0;
    }

    Display::handleEvent(event);
  }

  bool drawLine() {
    if (osd.shouldDraw()) {
      canvas = img;
      osd.draw(canvas);
      updateImage(canvas);
    }
    else
      updateImage(img);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_MOUSEBUTTONDOWN
	  || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
	SDL_PushEvent(&event);
	return true;
      }
      Display::handleEvent(event);
    }
    Display::update();
    return exitflag;
  }
  
  void render() {
    const double minpreview = 1.5e-16;
    bool hwflag = false;
    
    if (sz.re.get_d() >= minpreview && sz.im.get_d() >= minpreview) {
      hwflag = true;
      SDL_SetWindowTitle(window, "Rendering (hardware arithmetic)...");
    }

    //Both HW and Arb
    HPComplex pt;
    int its;
    Color color;
    Byte *Rp = img.R(), *Gp = img.G(), *Bp = img.B();

    //HW
    LPComplex Z, Z0;

    //Arb
    std::vector<Pt> probe_pts;
    std::vector<HPComplex> X, A, B, C;
    HPComplex probe, ref;

    //Probe for deepest reference orbit
    if (!hwflag) {
      for (int c = 0; c < img.nc; c++) {
	probe_pts.push_back(Pt(img.nr / 4, c));
	probe_pts.push_back(Pt(img.nr / 2, c));
	probe_pts.push_back(Pt(3 * img.nr / 4, c));
      }
      for (int r = 0; r < img.nr; r++)
	probe_pts.push_back(Pt(r, img.nc / 2));
      
      for (auto pt : probe_pts) {
	X.resize(N);
	probe.re = corner.re + pt.c * sz.re;
	probe.im = corner.im + (img.nr - pt.r - 1) * sz.im;
	getRefOrbit(X, probe);

	if (X.size() > A.size()) {
	  A = std::move(X);
	  ref = probe;
	}
      }

      printf("Deepest probe was %d / %d iterations\n", (int)A.size(), N);

      X = std::move(A);    
      getSeries(A, B, C, X);
    }
    
    if (drawlines) img = canvas;

    double smoothing;
    for (int r = 0, i = 0; r < img.nr; r++) {
      for (int c = 0; c < img.nc; c++, i++) {
	pt.re = corner.re + c * sz.re;
	pt.im = corner.im + (img.nr - r - 1) * sz.im;
	
	if (hwflag) {
	  Z.re = Z0.re = pt.re.get_d();
	  Z.im = Z0.im = pt.im.get_d();
	  for (its = 0; its < N; its++) {
	    Z = sq(Z) + Z0;
	    if (sqMag(Z) > bailout2) break;
	  }
	  if (its < N)
	    smoothing = getSmoothingMagnitude(Z);
	}
	else {
	  its = getIterations(X, A, B, C, ref, pt, N, smoothing);
	}

	if (its < N) {
	  if (smoothflag) {
	    color.r = interp(pal[its - 1].r, pal[its].r, smoothing);
	    color.g = interp(pal[its - 1].g, pal[its].g, smoothing);
	    color.b = interp(pal[its - 1].b, pal[its].b, smoothing);
	  }
	  else color = pal[its];
	}
	else
	  color = Color(0);

	Rp[i] = color.r; Gp[i] = color.g; Bp[i] = color.b;
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

    if (osd.shouldDraw()) redrawflag = true;
    
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

      osd.draw(canvas);
      
      updateImage(canvas);
      redrawflag = false;
    }

    if (!mousedown) frameDelay = 25;
    
    Display::update();
  }

  void reset() {
    constructDefaultPalette();
    
    renderflag = redrawflag = drawlines = true;
    smoothflag = false;

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
    int palN = (src.levels() - 1) * 256;
    pal = LinearPalette(src).cache(palN);
  }

  void constructNewPalette() {
    pal = getMultiWave(N);
  }
  
  void save() {
    std::string fn;
    if (!scanner.getString(canvas, "Enter a filename to save:", fn)) return;

    HPComplex center;
    center.re = corner.re + (img.nc / 2) * sz.re;
    center.im = corner.im + (img.nr / 2) * sz.im;
    
    FILE* fp = fopen(fn.c_str(), "w");
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
      osd.print("Saved to " + fn);
    }
    else {
      osd.print("Could not save to " + fn);
    }    
  }

  void load() {
    std::string fn;
    if (!scanner.getString(canvas, "Enter a filename to load:", fn)) return;

    char buf[10 * 1024];

    HPComplex center;
    FILE* fp = fopen(fn.c_str(), "r");
    if (fp) {
      fscanf(fp, "%d\n", &N);
      fscanf(fp, "%s\n", buf); center.re = buf;
      fscanf(fp, "%s\n", buf); center.im = buf;
      fscanf(fp, "%s\n", buf); sz.re = buf;
      fscanf(fp, "%s\n", buf); sz.im = buf;
      fclose(fp);

      corner.re = center.re - (img.nc / 2) * sz.re;
      corner.im = center.im - (img.nr / 2) * sz.im;
      
      renderflag = true;
      osd.print("Loaded from " + fn);
    }
    else {
      osd.print("Could not load from " + fn);
    }
  }
  
public:
  MyDisplay(int h, int w) : Display(h, w), img(h, w, 3), canvas(h, w) {
    for (int r = 0; r < h; r++)
      for (int c = 0; c < w; c++)
	canvas.at(r, c) = (((r / 4) + (c / 4)) & 1)? 255 : 192;
    canvas = canvas.toColor();

    osd.setColors(Color(255), Color(0));
    scanner.setColors(Color(255), Color(0));
    scanner.setDisplay(this);
    
    reset();
  }
};

int main(int argc, char* argv[]) {
  const int w = 800;
  const int h = 600;

  TextRenderer font("res/FreeSans.ttf", 20);
  OSD_Printer::setFont(&font);
  OSD_Scanner::setFont(&font);
  
  MyDisplay(h, w).main();
  
  return 0;
}
