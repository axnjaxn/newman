#include "display.h"

using namespace byteimage;

void MyDisplay::handleEvent(SDL_Event event) { 
  if (layout.widgets[0].widget == fractal) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      fractal->handleKeyEvent(event);
  }
  else if (layout.widgets[0].widget == editor) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);
    //TODO: Palette key events
  }

  WidgetDisplay::handleEvent(event);
}

void MyDisplay::update() {
  if (layout.widgets[0].widget == fractal)
    fractal->update();
  //TODO: Palette per-frame function?
      
  WidgetDisplay::update();
}
  
MyDisplay::MyDisplay() : WidgetDisplay(600, 800, "NewMandel"), font("res/FreeSans.ttf", 20) {
  fractal = new FractalRender(this, canvas.nr, canvas.nc);
  editor = new Editor(this);
  openFractal();
}
  
MyDisplay::~MyDisplay() {
  delete fractal;
}

//TODO: This group (don't forget to add OSD printing to update function)
bool MyDisplay::getInt(const std::string& prompt, int& v) { }

bool MyDisplay::getFloat(const std::string& prompt, float& v) { }

bool MyDisplay::getDouble(const std::string& prompt, double& v) { }  

bool MyDisplay::getString(const std::string& prompt, std::string& v) { }

void MyDisplay::print(const std::string& str, ...) {
  va_list args;
  va_start(args, str);
  char buf[1024];
  vsprintf(buf, str.c_str(), args);
  osd.print(str);
}

void MyDisplay::openFractal() {
  layout.clear();
  layout.attach(fractal, 0, 0, canvas.nc, canvas.nr, false);
}

void MyDisplay::openPalette() {
  layout.clear();
  layout.attach(editor, 0, 0, canvas.nc, canvas.nr, false);
}

int main(int argc, char* argv[]) {
  TextRenderer font("res/FreeSans.ttf", 20);
  OSD_Printer::setFont(&font);
  OSD_Scanner::setFont(&font);

  MyDisplay().main();
  return 0;
}
