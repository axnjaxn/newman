#ifndef _BPJ_NEWMANDEL_COMPLEX_H
#define _BPJ_NEWMANDEL_COMPLEX_H

#include <gmpxx.h>

class LPComplex {
public:
  double re, im;

  constexpr LPComplex() : re(0.0), im(0.0) { }
  constexpr LPComplex(double re, double im) : re(re), im(im) { }
};

class HPComplex {
public:
  mpf_class re, im;
};

constexpr LPComplex sq(const LPComplex& a) {
  return LPComplex(a.re * a.re - a.im * a.im, 2.0 * a.re * a.im);
}

constexpr double sqMag(const LPComplex& a) {return a.re * a.re + a.im * a.im;}

constexpr LPComplex operator+(const LPComplex& a, const LPComplex& b) {
  return LPComplex(a.re + b.re, a.im + b.im);
}

constexpr LPComplex operator*(const LPComplex& a, const LPComplex& b) {
  return LPComplex (a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re);
}

inline LPComplex descend(const HPComplex& a) {
  return LPComplex(a.re.get_d(), a.im.get_d());
}

#endif
