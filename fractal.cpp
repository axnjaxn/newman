#include "fractal.h"
#include "display.h"

using namespace byteimage;

void FractalViewer::save() {
  MyDisplay* display = (MyDisplay*)this->display;
  
  std::string fn;
  if (!display->getString("Enter a filename to save:", fn)) return;

  HPComplex corner, sz;
  mandel.getCorner(corner);
  sz = mandel.sz;
  
  HPComplex center;
  center.re = corner.re + (img.nc / 2) * sz.re;
  center.im = corner.im + (img.nr / 2) * sz.im;
    
  FILE* fp = fopen(fn.c_str(), "w");
  if (fp) {
    fprintf(fp, "%d\n", mandel.N);
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

  HPComplex center;
  FILE* fp = fopen(fn.c_str(), "r");
  if (fp) {
    fclose(fp);
    mandel.loadLegacy(fn.c_str());

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
  pal = mw.cache(mandel.N);
}

void FractalViewer::reset() {
  renderflag = drawlines = smoothflag = true;
  mousedown = 0;

  mandel = Mandelbrot(img.nr, img.nc);
  
  constructDefaultPalette();

  display->setRenderFlag();
}
  
void FractalViewer::recolor() {
  for (int r = 0; r < img.nr; r++)
    colorLine(r);
}

void FractalViewer::colorLine(int r) {
  RenderGrid::EscapeValue escape;
  Color color;
  for (int c = 0; c < img.nc; c++) {
    escape = mandel.at(r, c);
    if (mandel.at(r, c).iterations < mandel.N) {
      if (smoothflag)
	color = interp(pal[escape.iterations - 1],
		       pal[escape.iterations],
		       escape.smoothing);
      else
	color = pal[escape.iterations];
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
  if (mandel.useHardware())
    display->setTitle("Rendering (hardware arithmetic)...");
  
  mandel.precompute();
    
  if (drawlines) img = canvas;

  for (int r = 0; r < img.nr; r++) {
    mandel.computeRow(r);
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
      mandel.N += 256;
      pal = mw.cache(mandel.N);
      renderflag = true;
      display->print("%d iterations", mandel.N);
      break;
    case SDLK_DOWN:
      if (mandel.N > 256) mandel.N -= 256;
      recolor();
      display->setRenderFlag();
      display->print("%d iterations", mandel.N);
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
	if (n > mandel.N) {
	  pal = mw.cache(n);
	  renderflag = true;
	}
	else {
	  recolor();
	  display->setRenderFlag();
	}
	mandel.N = n;
	display->print("%d iterations.", mandel.N);
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
    HPComplex corner, sz;
    mandel.getCorner(corner);
    sz = mandel.sz;
    
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

    mandel.setCornerSize(corner, sz);

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
