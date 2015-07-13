#ifndef _BPJ_PALETTE_EDITOR_H
#define _BPJ_PALETTE_EDITOR_H

#include "multiwave.h"

#include <byteimage/widget.h>
#include <byteimage/font.h>

using byteimage::WidgetLayout;
using byteimage::WidgetDisplay;
using byteimage::TextRenderer;

class Button;//Fwd. Decl.
class Slider;//Fwd. Decl.

class Editor : public WidgetLayout {
protected:
  ByteImage bg;
  
  std::string filename;
  TextRenderer* font;

  Slider* slider;
  
  void resetMW();
  void load();
  void save();
  
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

  virtual void handleKeyEvent(SDL_Event event);
  virtual void render(ByteImage& target, int x, int y);

  void setBackground(const ByteImage& img);
};

#endif
