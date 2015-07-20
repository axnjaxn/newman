#include "video.h"

VideoZoom::VideoZoom()
  : nr(0), nc(0), rate(30) { }

void VideoZoom::start(const std::string& name, int nr, int nc, int rate) {
  writer.open(name, nr, nc, 30);
  img = ByteImage();
  this->nr = nr;
  this->nc = nc;
  this->rate = rate;
}

void VideoZoom::nextFrame(const ByteImage& img) {
  if (this->img.size()) {
    ByteImage canvas(nr, nc, 3), small, large;
    float t, v = pow(1.5, 1.0 / rate), sm = 2.0 / 3.0, lg = 1.0;
    for (int i = 0; i < rate; i++) {
      t = (float)i / rate;

      small = img.scaled((int)(img.nr * sm), (int)(img.nc * sm));
      large = this->img.scaled((int)(this->img.nr * lg), (int)(this->img.nc * lg));
      canvas.blit(large, (nr - large.nr) / 2, (nc - large.nc) / 2);
      canvas.blend(small, t, (nr - small.nr) / 2, (nc - small.nc) / 2);
      
      sm *= v;
      lg *= v;

      writer.write(canvas);
    }
  }

  this->img = img;
}
