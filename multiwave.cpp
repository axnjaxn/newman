#include "multiwave.h"

using namespace byteimage;

float MultiWaveGenerator::FloatCycle::value(int step) const {
  float t = values.size() * (step % period) / (float)period;
  int i0 = (int)t;
  int i1 = (i0 + 1) % values.size();
  t -= i0;

  return (1.0 - t) * values[i0] + t * values[i1];
}

float MultiWaveGenerator::FloatWave::value(int step) const {
  constexpr float tau = 2.0 * 3.14159265358979;
  return amplitude * sin(step * tau / period);
}

void MultiWaveGenerator::load_filename(const char* fn) {
  FILE* fp = fopen(fn, "r");
  if (!fp) return;

  int n;
  
  fscanf(fp, "%d", &n);
  hue_cycles.resize(n);
  for (int i = 0; i < hue_cycles.size(); i++) {
    fscanf(fp, "%d", &n);
    hue_cycles[i].values.resize(n);
    for (int j = 0; j < hue_cycles[i].values.size(); j++)
      fscanf(fp, "%f", &hue_cycles[i].values[j]);
    fscanf(fp, "%d", &hue_cycles[i].period);
  }
  fscanf(fp, "%d", &hue_period);
  
  fscanf(fp, "%d", &n);
  sat_cycle.values.resize(n);
  for (int i = 0; i < sat_cycle.values.size(); i++)
    fscanf(fp, "%f", &sat_cycle.values[i]);
  fscanf(fp, "%d", &sat_cycle.period);

  fscanf(fp, "%d", &n);
  lum_waves.resize(n);
  for (int i = 0; i < lum_waves.size(); i++)
    fscanf(fp, "%f %d", &lum_waves[i].amplitude, &lum_waves[i].period);
    
  fclose(fp);
}

void MultiWaveGenerator::save_filename(const char* fn) const {
  FILE* fp = fopen(fn, "w");
  if (!fp) return;

  fprintf(fp, "%d\n", (int)hue_cycles.size());
  for (int i = 0; i < hue_cycles.size(); i++) {
    fprintf(fp, "%d ", (int)hue_cycles[i].values.size());
    for (int j = 0; j < hue_cycles[i].values.size(); j++)
      fprintf(fp, "%f ", hue_cycles[i].values[j]);
    fprintf(fp, "%d\n", hue_cycles[i].period);
  }
  fprintf(fp, "%d\n\n", hue_period);
  
  fprintf(fp, "%d\n", (int)sat_cycle.values.size());
  for (int i = 0; i < sat_cycle.values.size(); i++)
    fprintf(fp, "%f ", sat_cycle.values[i]);
  fprintf(fp, "\n%d\n\n", sat_cycle.period);

  fprintf(fp, "%d\n", (int)lum_waves.size());
  for (int i = 0; i < lum_waves.size(); i++)
    fprintf(fp, "%f %d\n", lum_waves[i].amplitude, lum_waves[i].period);
     
  fclose(fp);
}

Color interp(const Color& c1, const Color& c2, float t) {
  return Color(interp(c1.r, c2.r, t),
	       interp(c1.g, c2.g, t),
	       interp(c1.b, c2.b, t));	       
}

CachedPalette MultiWaveGenerator::cache(int N) const {
  CachedPalette pal(N);

  float sat, lum;
  float ty, tx;
  int y0, y1, x0, x1;
  Color rgb0, rgb1;
  for (int i = 0; i < N; i++) {
    sat = sat_cycle.value(i);

    lum = 0.0;
    for (auto wave : lum_waves)
      lum += wave.value(i);
    lum = 1.0 / (1.0 + exp(-lum));//Logistic curve for asymptotic sum

    //ty interpolates between two hue cycles, c0 and c1
    ty = hue_cycles.size() * (i % hue_period) / (float)hue_period;
    y0 = (int)ty;
    y1 = (y0 + 1) % hue_cycles.size();
    ty -= y0;

    //Interpolate between values in c0
    tx = hue_cycles[y0].values.size() * (i % hue_cycles[y0].period) / (float)hue_cycles[y0].period;
    x0 = (int)tx;
    x1 = (x0 + 1) % hue_cycles[y0].values.size();
    tx -= x0;
    hsl2rgb(hue_cycles[y0].values[x0], sat, lum, rgb0.r, rgb0.g, rgb0.b);
    hsl2rgb(hue_cycles[y0].values[x1], sat, lum, rgb1.r, rgb1.g, rgb1.b);
    pal[i] = interp(rgb0, rgb1, tx);

    //Interpolate between values in c1 and result of c0 interpolation
    tx = hue_cycles[y1].values.size() * (i % hue_cycles[y1].period) / (float)hue_cycles[y1].period;
    x0 = (int)tx;
    x1 = (x0 + 1) % hue_cycles[y1].values.size();
    tx -= x0;
    hsl2rgb(hue_cycles[y1].values[x0], sat, lum, rgb0.r, rgb0.g, rgb0.b);
    hsl2rgb(hue_cycles[y1].values[x1], sat, lum, rgb1.r, rgb1.g, rgb1.b);    
    pal[i] = interp(pal[i], interp(rgb0, rgb1, tx), ty);
  }
  
  return pal;
}
