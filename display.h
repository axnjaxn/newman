#ifndef _BPJ_NEWMANDEL_DISPLAY_H
#define _BPJ_NEWMANDEL_DISPLAY_H

#include "fractal.h"
#include "editor.h"

#include <byteimage/osd.h>

using byteimage::OSD_Scanner;
using byteimage::OSD_Printer;

class MyDisplay : public WidgetDisplay {
protected:
  TextRenderer font;
  OSD_Scanner scanner;
  OSD_Printer osd;
  
  void handleEvent(SDL_Event event);
  void update();
  void render();
  
public:
  FractalViewer* fractal;
  Editor* editor;
  
  MyDisplay();  
  ~MyDisplay();

  bool forceUpdate();
  
  //Hooks to OSD_Scanner
  bool getInt(const std::string& prompt, int& v);
  bool getFloat(const std::string& prompt, float& v);
  bool getDouble(const std::string& prompt, double& v);  
  bool getString(const std::string& prompt, std::string& v);

  //Hooks to OSD_Printer
  void print(const std::string& str, ...);

  //Switching program modes
  void openFractal();  
  void openPalette();
};

#endif
