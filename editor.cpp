#include "editor.h"
#include "display.h"

#include <byteimage/render.h>

using namespace byteimage;

class Button : public Widget {
public:
  Editor* editor;
  Color bg = Color(255), fg = Color(0);
  std::string text;
  bool drawMinus = false, drawPlus = false;
  std::function<void()> fn;

  Button(Editor* editor) : Widget(editor->getDisplay()), editor(editor) { }

  virtual void render(ByteImage& canvas, int x, int y) {
    DrawRect(canvas, x, y, w, h, bg.r, bg.g, bg.b);
    if (!text.empty())
      editor->getFont()->drawCentered(canvas, text.c_str(), y + h / 2, x + w / 2, fg.r, fg.g, fg.b);
    if (drawMinus)
      DrawRect(canvas, x + w / 2 - 7, y + h / 2 - 1, 15, 3, fg.r, fg.g, fg.b);
    if (drawPlus)
      DrawCross(canvas, x + w / 2, y + h / 2, fg.r, fg.g, fg.b, 7, 3);
  }

  virtual void handleEvent(SDL_Event event) {    
    if (event.type == SDL_MOUSEBUTTONUP && fn) fn();
  }
};

class Slider : public Widget {
public:
  Editor* editor;
  LinearPalette pal;
  float value;
  std::function<void(float)> fn;
  
  Slider(Editor* editor, const LinearPalette& pal, float initial_value)
    : Widget(editor->getDisplay()), editor(editor), pal(pal), value(initial_value) { }

  virtual void render(ByteImage& canvas, int x, int y) {
    Color color;
    for (int c = 0; c < w; c++) {
      color = pal.inUnit((float)c / (w - 1));
      DrawRect(canvas, x + c, y, 1, h, color.r, color.g, color.b);
    }
    DrawRect(canvas, x + (int)(value * (w - 1) + 0.5), y, 1, h, 255);
  }

  virtual void handleEvent(SDL_Event event) {
    int x;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) x = event.button.x;
    else if (event.type == SDL_MOUSEMOTION && (event.motion.state & SDL_BUTTON_LMASK)) x = event.motion.x;
    else return;
    
    value = (float)x / (w - 1);
    if (fn) fn(value);

    display->setRenderFlag();
  }
};

Color getHue(float hue) {
  Color c;
  hsl2rgb(hue, 1.0, 0.5, c.r, c.g, c.b);
  return c;
}

Color getSat(float sat) {
  Color c;
  hsl2rgb(120.0, sat, 0.5, c.r, c.g, c.b);
  return c;
}

Color getLum(float lum) {
  Color c;
  hsl2rgb(0.0, 0.0, lum, c.r, c.g, c.b);
  return c;
}

WidgetLayout* HueCyclePreview(Editor* editor, int index) {
  WidgetLayout* layout = new WidgetLayout(editor->getDisplay());
  auto& cycle = editor->mw.hue_cycles[index];
  Button* button;
  int hue_index = 0;
  int pos = 0;
  int h = 32;

  button = new Button(editor);
  button->bg = Color(128); button->fg = Color(0);
  button->drawMinus = true;
  button->fn = [editor, index]() {editor->deleteCycle(index);};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  button = new Button(editor);
  button->bg = Color(192); button->fg = Color(0);
  button->drawPlus = true;
  button->fn = [editor, index]() {editor->addCycle(index);};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  button = new Button(editor);
  button->bg = Color(0); button->fg = Color(255);
  button->text = OSD_Printer::string("%d", cycle.period);
  button->fn = [editor, button, index]() {editor->changeHuePeriod(button, index);};
  layout->attach(button, pos, 0, h * 2, h);
  pos += h * 2;
  
  for (int i = 0; i < cycle.values.size(); i++) {
    button = new Button(editor);
    button->bg = getHue(cycle.values[i]);
    button->fn = [editor, button, index, i]() {editor->changeHue(button, index, i);};
    layout->attach(button, pos, 0, h, h);
    pos += h;
  }

  button = new Button(editor);
  button->bg = Color(128); button->fg = Color(0);
  button->drawMinus = true;
  button->fn = [editor, index]() {editor->deleteHue(index);};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  button = new Button(editor);
  button->bg = Color(192); button->fg = Color(0);
  button->drawPlus = true;
  button->fn = [editor, index]() {editor->addHue(index);};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  layout->updateSize(pos, h);

  return layout;
}

WidgetLayout* SatCyclePreview(Editor* editor) {
  WidgetLayout* layout = new WidgetLayout(editor->getDisplay());
  auto& cycle = editor->mw.sat_cycle;
  Button* button;
  int hue_index = 0;
  int pos = 0;
  int h = 32;

  for (int i = 0; i < cycle.values.size(); i++) {
    button = new Button(editor);
    button->bg = getSat(cycle.values[i]);
    button->fn = [editor, button, i]() {editor->changeSat(button, i);};
    layout->attach(button, pos, 0, h, h);
    pos += h;
  }

  button = new Button(editor);
  button->bg = Color(128); button->fg = Color(0);
  button->drawMinus = true;
  button->fn = [editor]() {editor->deleteSat();};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  button = new Button(editor);
  button->bg = Color(192); button->fg = Color(0);
  button->drawPlus = true;
  button->fn = [editor]() {editor->addSat();};
  layout->attach(button, pos, 0, h, h);
  pos += h;

  layout->updateSize(pos, h);

  return layout;
}

WidgetLayout* LumWavesPreview(Editor* editor) {
  WidgetLayout* layout = new WidgetLayout(editor->getDisplay());
  auto& waves = editor->mw.lum_waves;
  Button* button;
  int hue_index = 0;
  int h = 32;  

  for (int i = 0; i < waves.size(); i++) {
    button = new Button(editor);
    button->bg = getLum(waves[i].amplitude);
    button->fn = [editor, button, i]() {editor->changeLum(button, i);};
    layout->attach(button, 0, i * h, h, h);

    button = new Button(editor);
    button->bg = Color(0);
    button->fg = Color(255);
    button->text = OSD_Printer::string("%d", waves[i].period);
    button->fn = [editor, button, i]() {editor->changeLumPeriod(button, i);};
    layout->attach(button, h, i * h, 2 * h, h);
  }

  button = new Button(editor);
  button->bg = Color(128); button->fg = Color(0);
  button->drawMinus = true;
  button->fn = [editor]() {editor->deleteLum();};
  layout->attach(button, 0, waves.size() * h, h, h);

  button = new Button(editor);
  button->bg = Color(192); button->fg = Color(0);
  button->drawPlus = true;
  button->fn = [editor]() {editor->addLum();};
  layout->attach(button, h, waves.size() * h, h, h);

  layout->updateSize(3 * h, (waves.size() + 1) * h);

  return layout;
}

class MWPreview : public Widget {
protected:
  Editor* editor;
  
public:
  MWPreview(Editor* editor) : Widget(editor->getDisplay()), editor(editor) { }

  virtual void render(ByteImage& canvas, int x, int y) {
    CachedPalette pal = editor->mw.cache(w);
    for (int i = 0; i < w; i++)
      DrawRect(canvas, x + i, y, 1, h, pal[i].r, pal[i].g, pal[i].b);
  }
};

void Editor::resetMW() {
  mw = MultiWaveGenerator();
  mw.load_filename("default.pal");
  filename = "default.pal";

  closeSlider();
}

void Editor::load() {
  MyDisplay* display = (MyDisplay*)this->display;
  
  std::string fn;
  if (display->getString("Enter a filename to load:", fn)) {
    filename = fn;
    mw.load_filename(fn.c_str());
  }

  closeSlider();
}
  
void Editor::save() {
  MyDisplay* display = (MyDisplay*)this->display;
  
  std::string fn;
  if (display->getString("Enter a filename to save:", fn))
    mw.save_filename(fn.c_str());
}

void Editor::updateWidgets() {
  clear();

  Button* button;
  int pos = 0;
  
  for (int i = 0; i < mw.hue_cycles.size(); i++) {
    WidgetLayout* layout = HueCyclePreview(this, i);
    attach(layout, 0, pos, layout->width(), layout->height());
    pos += 32;
  }

  button = new Button(this);
  button->bg = Color(0); button->fg = Color(255);
  button->text = OSD_Printer::string("%d", mw.hue_period);
  button->fn = [this, button]() {this->changeCyclePeriod(button);};
  attach(button, 0, pos, 64, 32);
  pos += 32;

  pos += 16;

  {
    WidgetLayout* layout = SatCyclePreview(this);
    attach(layout, 0, pos, layout->width(), layout->height());
    pos += 32;
  }

  button = new Button(this);
  button->bg = Color(0); button->fg = Color(255);
  button->text = OSD_Printer::string("%d", mw.sat_cycle.period);
  button->fn = [this, button]() {this->changeSatPeriod(button);};
  attach(button, 0, pos, 64, 32);
  pos += 32;
  
  pos += 16;

  {
    WidgetLayout* layout = LumWavesPreview(this);
    attach(layout, 0, pos, layout->width(), layout->height());
  }

  if (slider)
    attach(slider, 0, h - 52, w, 32, false);
  
  attach(new MWPreview(this), 0, h - 20, w, 20);

  display->setRenderFlag();
}

Editor::Editor(WidgetDisplay* display) : WidgetLayout(display) {
  font = new TextRenderer("res/FreeSans.ttf", 10);

  slider = nullptr;

  resetMW();
}

Editor::~Editor() {
  delete slider;
  delete font;
}

void Editor::deleteCycle(int index) {
  if (mw.hue_cycles.size() <= 1) return;

  mw.hue_cycles.erase(mw.hue_cycles.begin() + index);
  closeSlider();
}

void Editor::addCycle(int index) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  display->print("Add cycle", 2000);

  MultiWaveGenerator::FloatCycle cycle;
  cycle.values.push_back(0.0);
  cycle.period = 256;
  
  mw.hue_cycles.insert(mw.hue_cycles.begin() + index + 1, cycle);
  closeSlider();
}

void Editor::changeCyclePeriod(Button* button) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  int period;
  if (display->getInt("Enter the number of iterations:", period)) {
    mw.hue_period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  display->setRenderFlag();
}

void Editor::changeHue(Button* button, int index, int hue_index) {
  openSlider(new Slider(this, LinearPalette::hue(), mw.hue_cycles[index].values[hue_index] / 360.0));
  slider->fn = [this, button, index, hue_index](float hue) {
    this->mw.hue_cycles[index].values[hue_index] = hue * 360.0;
    button->bg = getHue(hue * 360.0);
  };
}

void Editor::changeHuePeriod(Button* button, int index) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  int period;
  if (display->getInt("Enter the number of iterations:", period)) {
    mw.hue_cycles[index].period = period;
    button->text = OSD_Printer::string("%d", period);
  }
  
  display->setRenderFlag();
}

void Editor::deleteHue(int index) {
  if (mw.hue_cycles[index].values.size() <= 1) return;

  mw.hue_cycles[index].values.pop_back();
  closeSlider();
}

void Editor::addHue(int index) {
  mw.hue_cycles[index].values.push_back(mw.hue_cycles[index].values[mw.hue_cycles[index].values.size() - 1]);
  closeSlider();
}

void Editor::changeSat(Button* button, int sat_index) {
  openSlider(new Slider(this, LinearPalette(getSat(0.0), getSat(1.0)), mw.sat_cycle.values[sat_index]));
  slider->fn = [this, button, sat_index](float sat) {
    this->mw.sat_cycle.values[sat_index] = sat;
    button->bg = getSat(sat);
  };
}

void Editor::changeSatPeriod(Button* button) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  int period;
  if (display->getInt("Enter the number of iterations:", period)) {
    mw.sat_cycle.period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  display->setRenderFlag();
}

void Editor::deleteSat() {
  if (mw.sat_cycle.values.size() <= 1) return;

  mw.sat_cycle.values.pop_back();
  closeSlider();
}

void Editor::addSat() {
  mw.sat_cycle.values.push_back(mw.sat_cycle.values[mw.sat_cycle.values.size() - 1]);
  closeSlider();
}

void Editor::changeLum(Button* button, int lum_index) {
  openSlider(new Slider(this, LinearPalette(Color(0), Color(255)), mw.lum_waves[lum_index].amplitude));
  slider->fn = [this, button, lum_index](float lum) {
    this->mw.lum_waves[lum_index].amplitude = lum;
    button->bg = getLum(lum);
  };
}

void Editor::changeLumPeriod(Button* button, int lum_index) {
  MyDisplay* display = (MyDisplay*)this->display;
  
  int period;
  if (display->getInt("Enter the number of iterations:", period)) {
    mw.lum_waves[lum_index].period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  display->setRenderFlag();
}

void Editor::deleteLum() {
  if (mw.lum_waves.size() <= 1) return;

  mw.lum_waves.pop_back();
  closeSlider();
}

void Editor::addLum() {
  mw.lum_waves.push_back(mw.lum_waves[mw.lum_waves.size() - 1]);
  closeSlider();
}

void Editor::openSlider(Slider* slider) {
  remove(this->slider);
  this->slider = slider;
  attach(slider, 0, h - 52, w, 32, false);
  display->setRenderFlag();
}

void Editor::closeSlider() {
  delete slider;
  slider = nullptr;
  updateWidgets();
}

void Editor::handleKeyEvent(SDL_Event event) {
  //TODO: Unified multiwave
  
  MyDisplay* display = (MyDisplay*)this->display;
  
  if (event.type == SDL_KEYDOWN)
    switch (event.key.keysym.sym) {
    case SDLK_F2: save(); break;
    case SDLK_F3: load(); break;
    case SDLK_p: display->openFractal(); break;
    }
}
