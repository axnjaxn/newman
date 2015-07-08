#ifndef _BPJ_NEWMANDEL_DISPLAY_H
#define _BPJ_NEWMANDEL_DISPLAY_H

#include "fractal.h"

class MyDisplay : public WidgetDisplay {
protected:
  void handleEvent(SDL_Event event);
  void update();
  
public:
  FractalRender* fractal;
  
  MyDisplay();  
  ~MyDisplay();

  void openFractal();  
  void openPalette();
};

#endif
