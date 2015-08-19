#ifndef NUMERICS_H
#define NUMERICS_H

#include <math.h>
#include <float.h>
#include <assert.h>

template <class Real> class Numerics {};

template <> class Numerics<int> {
public:
  
  // Factorial
  static int factorial(int k) {
    int tmp = 1;
    if (k != 0) {
      for(int i = 1; i <= k; i++) {
	tmp *= i;
      }
    }
    return tmp;
  }
};

template <> class Numerics<float> {
public:
  
  // Rounding to floor
  static float rfloor(float k) { return floorf(k); }
  
  // Rounding to ceiling
  static float rceil(float k) { return ceilf(k); }
  
  // Absolute value (floating point number)
  static float fpabs(float k) { return fabsf(k); }
  
  // Square root
  static float sqroot(float k) { return sqrtf(k); }
  
  // Sine
  static float sine(float k) { return sinf(k); }
  
  // Cosine
  static float cosine(float k) { return cosf(k); }
  
  // Arc Cosine
  static float acosine(float k) { return acosf(k); }
  
  // e
  static float e(float k) { return expf(k); }
  
  // Power
  static float power(float k, float l) { return powf(k, l); }
  
  // Detection of nan/inf values
  static int isfinite(float k) { return finite( static_cast<double>(k) ); }
  
  // Numerical error threshold
  static float numthreshold() { return FLT_EPSILON; }
};

template <> class Numerics<long> {
public:
  
  // Factorial
  static long factorial(long k) {
    long tmp = 1;
    if (k != 0) {
      for(long i = 1; i <= k; i++) {
	tmp *= i;
      }
    }
    return tmp;
  }
};

template <> class Numerics<double> {
public:
  
  // Rounding to floor
  static double rfloor(double k) { return floor(k); }
  
  // Rounding to ceiling
  static double rceil(double k) { return ceil(k); }
  
  // Absolute value (floating point number)
  static double fpabs(double k) { return fabs(k); }
  
  // Square root
  static double sqroot(double k) { return sqrt(k); }
  
  // Sine
  static double sine(double k) { return sin(k); }
  
  // Cosine
  static double cosine(double k) { return cos(k); }
  
  // Arc Cosine
  static double acosine(double k) { return acos(k); }
  
  // e
  static double e(double k) { return exp(k); }
  
  // Power
  static double power(double k, double l) { return pow(k, l); }
  
  // Detection of nan/inf values
  static int isfinite(double k) { return finite(k); }
  
  // Numerical error threshold
  static double numthreshold() { return DBL_EPSILON; }
};

#endif // NUMERICS_H
