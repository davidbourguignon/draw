#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdio.h>//tmp
#include <vector>
#include <map>
#include <GL/glu.h>
#include "texload.h"
#include "vec2.h"

/* Probability distributions */
class Probability_Distribution_Function {
protected:
  typedef long   integer;
  typedef double real;
  
public:
  virtual real operator()(const real x) const = 0;
};

class Gauss : public Probability_Distribution_Function {
private:
  real mu, sigma; // Parameters
  real A, B;      // Implementation commodities
  
public:
  Gauss();
  Gauss(const real u = 0.0, const real s = 1.0);
  real operator()(const real x) const;
};

class Gamma : public Probability_Distribution_Function {
private:
  integer alpha, theta; // Parameters
  real A, B;            // Implementation commodities
  
public:
  Gamma();
  Gamma(const integer a, const integer t);
  real operator()(const real x) const;
};

/* Texture */
class Texture {
private:
  typedef double real;
  
  bool isPowerOfTwo(const GLint size) const;
  void eval1DProbDistrib(const Probability_Distribution_Function& P,
			 std::vector<real>& function,
			 const int first, const int size) const;
  void filterImageBorders(std::vector<GLfloat>& image, const int pixel_size,
			  const int width, const int height) const;
  void buildMipmaps(const Probability_Distribution_Function& P,
		    const GLint symmetry, const int filter_type) const;
  
  std::vector<GLsizei> dimensions;
  
public:
  enum imageformat {SGI_ALPHA, SGI_RGB, SGI_RGBA};
  enum textureformat {ALPHA, RGB, RGBA};
  enum filtertype {NO_FILTER, CIRCULAR_GAUSSIAN_FILTER};
  
  Texture();
  Texture(char* file_name, const int image_format = SGI_RGB,
	  const int texture_format = RGB);
  Texture(const Probability_Distribution_Function& P,
	  const GLint symmetry, const std::vector<GLsizei>& dims,
	  const int filter_type = NO_FILTER);
  
  GLuint name;
};

#endif // TEXTURE_H
