#ifndef _BPJ_NEWMAN_VIEWER_H
#define _BPJ_NEWMAN_VIEWER_H

#include "mandelbrot.h"
#include "multiwave.h"
#include "video.h"
#include <byteimage/osd.h>
#include <byteimage/widget.h>


using byteimage::Color;
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
  bool zoomflag;   //Constant zoom

  //For autozoom
  HPComplex saved_center;
  VideoZoom zoom;

  //Numerical results of latest render
  Mandelbrot mandel;
  int sc;

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

  void initAutoZoom();
  
  void recolor();
  Color getColor(const RenderGrid::EscapeValue& escape);
  void colorLine(int r);
  bool drawLine(int r);
  void render();
  void beautyRender();

  void update();
  
public:  
  FractalViewer(WidgetDisplay* display, int h, int w);

  virtual void handleKeyEvent(SDL_Event event);
  virtual void handleEvent(SDL_Event event);
  virtual void render(ByteImage& canvas, int x, int y);
  
  friend class MyDisplay;
};

#endif
