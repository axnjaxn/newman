#include "display.h"

using namespace byteimage;

void MyDisplay::handleEvent(SDL_Event event) {
  if (layout.widgets[0].widget == fractal) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      fractal->handleKeyEvent(event);
  }
  //TODO: Palette key events

  WidgetDisplay::handleEvent(event);
}

void MyDisplay::update() {
  if (layout.widgets[0].widget == fractal) {
    fractal->update();
  }
      
  WidgetDisplay::update();
}
  
MyDisplay::MyDisplay() : WidgetDisplay(600, 800, "NewMandel") {
  fractal = new FractalRender(this, canvas.nr, canvas.nc);
  openFractal();
}
  
MyDisplay::~MyDisplay() {
  delete fractal;
}

void MyDisplay::openFractal() {
  layout.clear();
  layout.attach(fractal, 0, 0, canvas.nc, canvas.nr, false);
}
  
void MyDisplay::openPalette() { }//TODO

int main(int argc, char* argv[]) {
  TextRenderer font("res/FreeSans.ttf", 20);
  OSD_Printer::setFont(&font);
  OSD_Scanner::setFont(&font);

  MyDisplay().main();
  return 0;
}
