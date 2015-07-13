#include "fractal.h"
#include "display.h"

using namespace byteimage;

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
		  float& smoothing) {
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

void FractalViewer::save() {
  MyDisplay* display = (MyDisplay*)this->display;
  
  std::string fn;
  if (!display->getString("Enter a filename to save:", fn)) return;

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
    display->print("Saved to " + fn);
  }
  else {
    display->print("Could not save to " + fn);
  }
}

void FractalViewer::load() {
  MyDisplay* display = (MyDisplay*)this->display;
  
  std::string fn;
  if (!display->getString("Enter a filename to load:", fn)) return;

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

    updatePalette();
      
    renderflag = true;
    display->print("Loaded from " + fn);
  }
  else {
    display->print("Could not load from " + fn);
  }
}

void FractalViewer::screenshot() {
  MyDisplay* display = (MyDisplay*)this->display;

  char fn[256];
  int t = (int)time(NULL);
  sprintf(fn, "%d.png", t);
  img.save_filename(fn);
  display->print(OSD_Printer::string("Saved screenshot to %s", fn));
}

void FractalViewer::constructDefaultPalette() {
  mw.load_filename("default.pal");
  updatePalette();
}

void FractalViewer::updatePalette() {
  pal = mw.cache(N);
}

void FractalViewer::reset() {
  renderflag = drawlines = smoothflag = true;
  mousedown = 0;

  N = 256;
  constructDefaultPalette();
    
  corner.re = -2.5;
  corner.im = -1.5;
    
  sz.re = (1.5 - corner.re) / img.nc;
  sz.im = (1.5 - corner.im) / img.nr;

  display->setRenderFlag();
}
  
void FractalViewer::recolor() {
  for (int r = 0; r < img.nr; r++)
    colorLine(r);
}

void FractalViewer::colorLine(int r) {
  Color color;
  for (int c = 0; c < img.nc; c++) {
    if (grid.at(r, c).iterations < N) {
      if (smoothflag)
	color = interp(pal[grid.at(r, c).iterations - 1],
		       pal[grid.at(r, c).iterations],
		       grid.at(r, c).smoothing);
      else
	color = pal[grid.at(r, c).iterations];
    }
    else color = Color(0);
      
    img.at(r, c, 0) = color.r;
    img.at(r, c, 1) = color.g;
    img.at(r, c, 2) = color.b;
  }
}

bool FractalViewer::drawLine(int r) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  colorLine(r);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_MOUSEBUTTONDOWN
	|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
	|| (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
	|| event.type == SDL_QUIT) {
      SDL_PushEvent(&event);
      return true;
    }
  }

  display->setRenderFlag();
  return display->forceUpdate();
}

void FractalViewer::render() {
  const double minpreview = 1.5e-16;
  bool hwflag = false;
    
  if (sz.re.get_d() >= minpreview && sz.im.get_d() >= minpreview) {
    hwflag = true;
    display->setTitle("Rendering (hardware arithmetic)...");
  }

  //Both HW and Arb
  HPComplex pt;
  int its;

  //HW
  LPComplex Z, Z0;

  //Arb
  std::vector<Pt> probe_pts;
  std::vector<HPComplex> X, A, B, C;
  HPComplex probe, ref;

  //Probe for deepest reference orbit
  if (!hwflag) {
    for (int c = 0; c < img.nc; c += 2) {
      probe_pts.push_back(Pt(img.nr / 4, c));
      probe_pts.push_back(Pt(img.nr / 2, c));
      probe_pts.push_back(Pt(3 * img.nr / 4, c));
    }
    for (int r = 0; r < img.nr; r += 2)
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
	if (its < N) {
	  grid.at(r, c).iterations = its;
	  grid.at(r, c).smoothing = getSmoothingMagnitude(Z);
	}
	else {
	  grid.at(r, c).iterations = N;
	  grid.at(r, c).smoothing = 0.0;
	}
      }
      else {
	grid.at(r, c).iterations = getIterations(X, A, B, C, ref, pt, N, grid.at(r, c).smoothing);
      }
    }

    if (drawlines && drawLine(r)) break;
  }

  display->setRenderFlag();
}

void FractalViewer::update() {
  display->frameDelay = 0;
    
  if (renderflag) {
    display->setTitle("Rendering...");
    Uint32 ticks = SDL_GetTicks();
    render();
    ticks = SDL_GetTicks() - ticks;
    char str[256];
    sprintf(str, "Time: %dms", ticks);
    display->setTitle(str);
    
    renderflag = false;
    display->setRenderFlag();
  }

  if (!mousedown) display->frameDelay = 25;
}

FractalViewer::FractalViewer(WidgetDisplay* display, int h, int w) : Widget(display) {
  canvas = ByteImage(h, w);
  for (int r = 0; r < h; r++)
    for (int c = 0; c < w; c++)
      canvas.at(r, c) = (((r / 4) + (c / 4)) & 1)? 255 : 192;
  canvas = img = canvas.toColor();
  grid = RenderGrid(h, w);
  
  reset();
}

void FractalViewer::handleKeyEvent(SDL_Event event) {
  MyDisplay* display = (MyDisplay*)this->display;
  int n;
  if (event.type == SDL_KEYDOWN)
    switch (event.key.keysym.sym) {
    case SDLK_F5: display->setRenderFlag(); break;
    case SDLK_BACKSPACE: reset(); break;
    case SDLK_RETURN: renderflag = true; break;
    case SDLK_UP:
      N += 256;
      pal = mw.cache(N);
      renderflag = true;
      display->print("%d iterations", N);
      break;
    case SDLK_DOWN:
      if (N > 256) N -= 256;
      recolor();
      display->setRenderFlag();
      display->print("%d iterations", N);
      break;
    case SDLK_F2: save(); break;
    case SDLK_F3: load(); break;
    case SDLK_p:
      display->openPalette();
      break;
    case SDLK_F11: screenshot(); break;
    case SDLK_s:
      smoothflag = !smoothflag;
      display->print("Smoothing: %s", smoothflag? "on" : "off");
      renderflag = true;
      break;
    case SDLK_d:
      drawlines = !drawlines;
      display->print("Draw lines mode: %s", drawlines? "on" : "off");
      break;
    case SDLK_i:
      if (display->getInt("How many iterations?", n)) {
	if (n > N) {
	  pal = mw.cache(n);
	  renderflag = true;
	}
	else {
	  recolor();
	  display->setRenderFlag();
	}
	N = n;
	display->print("%d iterations.", N);
      }
      break;
    }
}

void FractalViewer::handleEvent(SDL_Event event) {
  if (event.type == SDL_MOUSEBUTTONDOWN) {
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
      display->setRenderFlag();
    }
    else if (mousedown == 2) {
      nx = event.button.x;
      ny = event.button.y;
      display->setRenderFlag();
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
}

void FractalViewer::render(ByteImage& canvas, int x, int y) {
  if (mousedown == 1){
    this->canvas.fill(255);
    this->canvas.blitSampled(img, scale, scale, my - scale * my, mx - scale * mx);
  }
  else if (mousedown == 2) {
    this->canvas.fill(255);
    this->canvas.blit(img, ny - my, nx - mx);
  }
  else {
    this->canvas = img;
  }

  canvas.blit(this->canvas, y, x);
}
