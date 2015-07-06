#ifndef _BPJ_PALETTE_EDITOR_H
#define _BPJ_PALETTE_EDITOR_H

#include "multiwave.h"

#include <byteimage/widget.h>
#include <byteimage/osd.h>

using byteimage::WidgetDisplay;
using byteimage::TextRenderer;
using byteimage::OSD_Printer;
using byteimage::OSD_Scanner;

class Editor : public WidgetDisplay {
protected:
  TextRenderer* font;
  OSD_Printer osd;
  OSD_Scanner scanner;
  
  void resetMW();
  void load();
  void save();

  virtual void handleEvent(SDL_Event event);
  virtual void update();
  
public:  
  MultiWaveGenerator mw;
  
  Editor();
  virtual ~Editor();

  void updateWidgets();

  inline TextRenderer* getFont() {return font;}//TODO
  
  void deleteCycle(int index);
  void addCycle(int index);
  void changeCyclePeriod();//TODO
  
  void changeHue(int index, int hue_index);
  void changeHuePeriod(int index);
  void deleteHue(int index);
  void addHue(int index);

  void changeSat(int sat_index);
  void changeSatPeriod();
  void deleteSat();
  void addSat();
  
  void changeLum(int lum_index);
  void changeLumPeriod(int lum_index);
  void deleteLum();
  void addLum();

  //TODO: Better hue, sat, lum editors
  //TODO: Finish numerical editors
};

#endif
