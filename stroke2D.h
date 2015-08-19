#ifndef STROKE2D_H
#define STROKE2D_H

#include "input.h"
#include "bezier.h"

class Stroke2D {
public:
  typedef GLdouble                      real;
  typedef Vec2<real>                    vec2;
  typedef Bezier_Augmented<real, vec2>  bezier;
  typedef std::vector<bezier>           beziers;
private:
  typedef Point<real, vec2>             point;
  typedef std::vector<point>            points;
  
  /*
   *  Main source of inspiration for Bezier fitting:
   *  An Algorithm for Automatically Fitting Digitized Curves
   *  by Philip J. Schneider
   *  from Graphics Gems, Academic Press, 1990.
   */
  void fitCurve(const points::iterator first, const points::iterator last,
		real error);
  void fitCubic(const points::iterator first, const points::iterator last,
		const vec2& tanv1, const vec2& tanv2, real error);
  void generateBezier(const points::const_iterator first,
		      const points::const_iterator last,
	              const vec2& tanv1, const vec2& tanv2, bezier& b);
  void findNewtonRaphsonRoot(bezier& Q, point& P);
  void computeMaxError(const points::iterator first,
		       const points::iterator last, bezier& b,
		       points::iterator& split, real* maxError);
  vec2 leftTangent(const points::const_iterator p,
		   const points::difference_type npoints);
  vec2 rightTangent(const points::const_iterator p,
		    const points::difference_type npoints);
  vec2 centerTangent(const points::const_iterator p);
  void chordLengthParameterize(const points::iterator first,
			       const points::iterator last);
  void reparameterize(const points::iterator first,
		      const points::iterator last, bezier& b);
  
  /* Misc */
  void fit(Input& in, const real error);
  void evalLength();
  bool testLength(const beziers::difference_type nb_first, const real t_first,
		  const beziers::difference_type nb_last, const real t_last);
  void getBoundingBox(const vec2& pt_first,
		      const beziers::difference_type nb_first,
		      const real t_first, const vec2& pt_last,
		      const beziers::difference_type nb_last,
		      const real t_last, vec2& min, vec2& max);
  
  static const real steps_per_unit_length;
  
#if TEST_STROKE2D
public:
#endif
  void evalCurvatureCenters();
  
public:
  Stroke2D();
  Stroke2D(Input& in, const real error = 4.0);
  void read(std::ifstream& file_in);
  void write(std::ofstream& file_out) const;
  bool empty() const;
  void drawSpline() const;
  void drawControlPoints() const;
  void drawTangents() const;
  void drawDerivatives() const;
  void drawCurvatureVectors() const;
  
  beziers bs;
  real length;
  std::vector<real> relative_lengths;
};

#endif // STROKE2D_H
