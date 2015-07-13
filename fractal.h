#ifndef _BPJ_NEWMANDEL_FRACTAL_H
#define _BPJ_NEWMANDEL_FRACTAL_H

#include "render.h"
#include "multiwave.h"
#include <byteimage/osd.h>
#include <byteimage/widget.h>

using byteimage::ByteImage;
using byteimage::OSD_Scanner;
using byteimage::Widget;
using byteimage::WidgetDisplay;

class FractalViewer : public Widget {
protected:
  //Options
  bool renderflag; //Requires recomputation
  bool drawlines;  //This forces progress display
  bool smoothflag; //Smooth coloring

  //Numerical results of latest render
  Mandelbrot mandel;

  //For render output and display
  ByteImage img, canvas;
  
  //For coloring
  MultiWaveGenerator mw;
  CachedPalette pal;
  
  //For interactive movement
  int mousedown, mx, my, nx, ny;
  float scale;
  
  void save();
  void load();
  void screenshot();

  void constructDefaultPalette();
  void updatePalette();
  void reset();
  
  void recolor();
  void colorLine(int r);
  bool drawLine(int r);
  void render();

  void update();
  
public:  
  FractalViewer(WidgetDisplay* display, int h, int w);

  virtual void handleKeyEvent(SDL_Event event);
  virtual void handleEvent(SDL_Event event);
  virtual void render(ByteImage& canvas, int x, int y);
  
  friend class MyDisplay;
};

#endif
