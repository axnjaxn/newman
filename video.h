#ifndef _BPJ_NEWMAN_VIDEO_H
#define _BPJ_NEWMAN_VIDEO_H

#include <byteimage/video.h>

using byteimage::ByteImage;
using byteimage::VideoWriter;

/*
 * Constraints: this will only work with 1.5-ratio scaling for now
 */

class VideoZoom {
protected:
  VideoWriter writer;
  ByteImage img;
  int nr, nc;
  int rate;//Frames to interpolate per zoom
  
public:
  VideoZoom();
  
  void start(const std::string& name, int nr, int nc, int rate);
  void nextFrame(const ByteImage& img);
};

#endif
