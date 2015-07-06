#include "editor.h"
#include <byteimage/render.h>

using namespace byteimage;

class Button : public Widget {
public:
  Editor* editor;
  Color bg = Color(255), fg = Color(0);
  std::string text;
  bool drawMinus = false, drawPlus = false;
  std::function<void()> fn;

  Button(Editor* editor) : Widget(editor), editor(editor) { }

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
  WidgetLayout* layout = new WidgetLayout(editor);
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
  WidgetLayout* layout = new WidgetLayout(editor);
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
  WidgetLayout* layout = new WidgetLayout(editor);
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
public:
  MWPreview(WidgetDisplay* display) : Widget(display) { }

  virtual void render(ByteImage& canvas, int x, int y) {
    CachedPalette pal = ((Editor*)display)->mw.cache(w);
    for (int i = 0; i < w; i++)
      DrawRect(canvas, x + i, y, 1, h, pal[i].r, pal[i].g, pal[i].b);
  }
};

void Editor::resetMW() {
  mw = MultiWaveGenerator();
  mw.load_filename("default.pal");
  filename = "default.pal";

  updateWidgets();
}

void Editor::commit() {
  mw.save_filename(filename.c_str());
  osd.print("Committed changes.");
}

void Editor::load() {
  std::string fn;
  if (scanner.getString(canvas, "Enter a filename to load:", fn)) {
    filename = fn;
    mw.load_filename(fn.c_str());
  }

  updateWidgets();
}
  
void Editor::save() {
  std::string fn;
  if (scanner.getString(canvas, "Enter a filename to save:", fn))
    mw.save_filename(fn.c_str());
}

void Editor::updateWidgets() {
  layout.clear();

  Button* button;
  int pos = 0;
  
  for (int i = 0; i < mw.hue_cycles.size(); i++) {
    WidgetLayout* layout = HueCyclePreview(this, i);
    this->layout.attach(layout, 0, pos, layout->width(), layout->height());
    pos += 32;
  }

  button = new Button(this);
  button->bg = Color(0); button->fg = Color(255);
  button->text = OSD_Printer::string("%d", mw.hue_period);
  button->fn = [this, button]() {this->changeCyclePeriod(button);};
  layout.attach(button, 0, pos, 64, 32);
  pos += 32;

  pos += 16;

  {
    WidgetLayout* layout = SatCyclePreview(this);
    this->layout.attach(layout, 0, pos, layout->width(), layout->height());
    pos += 32;
  }

  button = new Button(this);
  button->bg = Color(0); button->fg = Color(255);
  button->text = OSD_Printer::string("%d", mw.sat_cycle.period);
  button->fn = [this, button]() {this->changeSatPeriod(button);};
  layout.attach(button, 0, pos, 64, 32);
  pos += 32;
  
  pos += 16;

  {
    WidgetLayout* layout = LumWavesPreview(this);
    this->layout.attach(layout, 0, pos, layout->width(), layout->height());
  }
  
  layout.attach(new MWPreview(this), 0, canvas.nr - 20, canvas.nc, 20);

  renderflag = true;
}

void Editor::handleEvent(SDL_Event event) {
  if (event.type == SDL_KEYDOWN)
    switch (event.key.keysym.sym) {
    case SDLK_F2: save(); break;
    case SDLK_F3: load(); break;
    case SDLK_RETURN: commit(); break;      
    }
  WidgetDisplay::handleEvent(event);
}

void Editor::update() {
  if (renderflag || osd.shouldDraw()) {
    render();
    osd.draw(canvas);
    updateImage(canvas);
    renderflag = false;    
  }
  Display::update();
}

Editor::Editor() : WidgetDisplay(600, 800) {
  font = new TextRenderer("res/FreeSans.ttf", 10);
  
  osd.setColors(Color(255), Color(0));
  scanner.setColors(Color(255), Color(0));
  scanner.setDisplay(this);

  frameDelay = 25;
  
  resetMW();
}

Editor::~Editor() {
  delete font;
}

void Editor::deleteCycle(int index) {
  if (mw.hue_cycles.size() <= 1) return;

  mw.hue_cycles.erase(mw.hue_cycles.begin() + index);
  updateWidgets();
}

void Editor::addCycle(int index) {
  osd.print("Add cycle", 2000);

  MultiWaveGenerator::FloatCycle cycle;
  cycle.values.push_back(0.0);
  cycle.period = 256;
  
  mw.hue_cycles.insert(mw.hue_cycles.begin() + index + 1, cycle);
  updateWidgets();
}

void Editor::changeCyclePeriod(Button* button) {
  int period;
  if (scanner.getInt(canvas, "Enter the number of iterations:", period)) {
    mw.hue_period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  renderflag = true;
}

void Editor::changeHue(Button* button, int index, int hue_index) {
  float hue;
  if (scanner.getFloat(canvas, "Enter a hue:", hue)) {
    mw.hue_cycles[index].values[hue_index] = hue;
    button->bg = getHue(hue);
  }

  renderflag = true;  
}

void Editor::changeHuePeriod(Button* button, int index) {
  int period;
  if (scanner.getInt(canvas, "Enter the number of iterations:", period)) {
    mw.hue_cycles[index].period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  renderflag = true;
}

void Editor::deleteHue(int index) {
  if (mw.hue_cycles[index].values.size() <= 1) return;

  mw.hue_cycles[index].values.pop_back();
  updateWidgets();
}

void Editor::addHue(int index) {
  mw.hue_cycles[index].values.push_back(mw.hue_cycles[index].values[mw.hue_cycles[index].values.size() - 1]);
  updateWidgets();
}

void Editor::changeSat(Button* button, int sat_index) {
  float sat;
  if (scanner.getFloat(canvas, "Enter a saturation:", sat)) {
    mw.sat_cycle.values[sat_index] = sat;
    button->bg = getSat(sat);
  }

  renderflag = true;
}

void Editor::changeSatPeriod(Button* button) {
  int period;
  if (scanner.getInt(canvas, "Enter the number of iterations:", period)) {
    mw.sat_cycle.period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  renderflag = true;
}

void Editor::deleteSat() {
  if (mw.sat_cycle.values.size() <= 1) return;

  mw.sat_cycle.values.pop_back();
  updateWidgets();
}

void Editor::addSat() {
  mw.sat_cycle.values.push_back(mw.sat_cycle.values[mw.sat_cycle.values.size() - 1]);
  updateWidgets();
}

void Editor::changeLum(Button* button, int lum_index) {
  float lum;
  if (scanner.getFloat(canvas, "Enter a luminance:", lum)) {
    mw.lum_waves[lum_index].amplitude = lum; 
    button->bg = getLum(lum);
  }

  renderflag = true;
}

void Editor::changeLumPeriod(Button* button, int lum_index) {
  int period;
  if (scanner.getInt(canvas, "Enter the number of iterations:", period)) {
    mw.lum_waves[lum_index].period = period;
    button->text = OSD_Printer::string("%d", period);
  }

  renderflag = true;
}

void Editor::deleteLum() {
  if (mw.lum_waves.size() <= 1) return;

  mw.lum_waves.pop_back();
  updateWidgets();
}

void Editor::addLum() {
  mw.lum_waves.push_back(mw.lum_waves[mw.lum_waves.size() - 1]);
  updateWidgets();
}
