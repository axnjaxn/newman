#include "display.h"

using namespace byteimage;

void MyDisplay::handleEvent(SDL_Event event) { 
  if (layout.widgets[0].widget == fractal) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      fractal->handleKeyEvent(event);
  }
  else if (layout.widgets[0].widget == editor) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      editor->handleKeyEvent(event);
  }

  WidgetDisplay::handleEvent(event);
}

void MyDisplay::update() {
  if (layout.widgets[0].widget == fractal)
    fractal->update();

  if (osd.shouldDraw()) {setRenderFlag();}
  
  WidgetDisplay::update();
}

void MyDisplay::render() {
  WidgetDisplay::render();
  osd.draw(canvas);
}
  
MyDisplay::MyDisplay() : WidgetDisplay(600, 800, "NewMandel"), font("res/FreeSans.ttf", 20) {
  fractal = new FractalRender(this, canvas.nr, canvas.nc);
  editor = new Editor(this);

  OSD_Printer::setFont(&font);
  OSD_Scanner::setFont(&font);
  osd.setColors(Color(255), Color(0));
  scanner.setColors(Color(255), Color(0));
  scanner.setDisplay(this);

  openFractal();
}
  
MyDisplay::~MyDisplay() {
  delete fractal;
}

bool MyDisplay::getInt(const std::string& prompt, int& v) {
  osd.hide();
  return scanner.getInt(canvas, prompt, v);
}

bool MyDisplay::getFloat(const std::string& prompt, float& v) {
  osd.hide();
  return scanner.getFloat(canvas, prompt, v);
}

bool MyDisplay::getDouble(const std::string& prompt, double& v) {
  osd.hide();
  return scanner.getDouble(canvas, prompt, v);
}

bool MyDisplay::getString(const std::string& prompt, std::string& v) {
  osd.hide();
  return scanner.getString(canvas, prompt, v);
}

void MyDisplay::print(const std::string& str, ...) {
  va_list args;
  va_start(args, str);
  char buf[1024];
  vsprintf(buf, str.c_str(), args);
  osd.print(buf);
}

void MyDisplay::openFractal() {
  layout.clear();
  layout.attach(fractal, 0, 0, canvas.nc, canvas.nr, false);
  setRenderFlag();
  //TODO: Shared multiwave palette
}

void MyDisplay::openPalette() {
  layout.clear();
  layout.attach(editor, 0, 0, canvas.nc, canvas.nr, false);
  setRenderFlag();
}

int main(int argc, char* argv[]) {
  MyDisplay().main();
  return 0;
}
