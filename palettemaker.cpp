#include "editor.h"

using namespace byteimage;

int main(int argc, char* argv[]) {
  TextRenderer font("res/FreeSans.ttf", 20);
  OSD_Printer::setFont(&font);
  OSD_Scanner::setFont(&font);
  
  Editor().main();
  return 0;
}
