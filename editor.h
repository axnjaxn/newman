#ifndef _BPJ_PALETTE_EDITOR_H
#define _BPJ_PALETTE_EDITOR_H

#include "multiwave.h"

#include <byteimage/widget.h>
#include <byteimage/osd.h>

using byteimage::WidgetLayout;
using byteimage::WidgetDisplay;
using byteimage::TextRenderer;
using byteimage::OSD_Printer;
using byteimage::OSD_Scanner;

class Button;//Fwd. Decl.
class Slider;//Fwd. Decl.

class Editor : public WidgetLayout {
protected:
  std::string filename;
  TextRenderer* font;
  OSD_Printer osd;
  OSD_Scanner scanner;

  Slider* slider;
  
  void resetMW();
  void commit();
  void load();
  void save();

  virtual void handleEvent(SDL_Event event);
  virtual void update();
  
public:  
  MultiWaveGenerator mw;
  
  Editor(WidgetDisplay* display);
  virtual ~Editor();

  void updateWidgets();

  inline TextRenderer* getFont() {return font;}//TODO
  
  void deleteCycle(int index);
  void addCycle(int index);
  void changeCyclePeriod(Button* button);
  
  void changeHue(Button* button, int index, int hue_index);
  void changeHuePeriod(Button* button, int index);
  void deleteHue(int index);
  void addHue(int index);

  void changeSat(Button* button, int sat_index);
  void changeSatPeriod(Button* button);
  void deleteSat();
  void addSat();
  
  void changeLum(Button* button, int lum_index);
  void changeLumPeriod(Button* button, int lum_index);
  void deleteLum();
  void addLum();

  void openSlider(Slider* slider);
  void closeSlider();
};

#endif
