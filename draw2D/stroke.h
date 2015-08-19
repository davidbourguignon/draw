#ifndef STROKE_H
#define STROKE_H

#include "input.h"

class Stroke {
  typedef double     real;
  typedef Vec2<real> vec2;
  
  // Parametric point
  class Point {
  public:
    Point();
    Point(const vec2& v);
    
    vec2 pos;
    real u; // Parameter value
  };
  typedef std::vector<Point> points;
  
  // Cubic bezier curve
  class Bezier {
  public:
    typedef std::vector<vec2> ctrl_points;
    
    Bezier();
    Bezier(int degree);
    Bezier(const Bezier& b);
    real B30(real u) const;
    real B31(real u) const;
    real B32(real u) const;
    real B33(real u) const;
    void evaluate(real t, vec2& Q) const; // de Casteljau's algorithm
    void evalDerivative(int order, real t, vec2& Q) const;
    bool evalCurvatureVector(real t, vec2& Q, vec2& C) const;
    
    ctrl_points V; // Control points
    real length;
  };
  typedef std::vector<Bezier> beziers;
  
  // Used in fitting algorithm
  class A {
  public:
    A() {}
    A(const vec2& v1, const vec2& v2) : A1(v1), A2(v2) {}
    
    vec2 A1, A2;
  };
  
  class Curvature_Vector {
  public:
    Curvature_Vector() {}
    Curvature_Vector(const vec2& q, const vec2& c, bool is_nb) : Q(q), C(c) {
      v = C - Q;
      R = v.norm();
      if (is_nb)
	if (R > 1.0e+3) // Magic number!
	  is_finite = false;
        else
	  is_finite = true;
      else
	is_finite = false;
    }
    
    vec2 Q, C, v;
    real R;
    bool is_finite;
  };
  
  void fitCurve(points::iterator first, points::iterator last, real error);
  void fitCubic(points::iterator first, points::iterator last,
		vec2 tanv1, vec2 tanv2, real error);
  void generateBezier(points::const_iterator first,
		      points::const_iterator last,
	              vec2 tanv1, vec2 tanv2, Bezier& b);
  void findNewtonRaphsonRoot(Bezier& Q, Point& P);
  void computeMaxError(points::iterator first, points::iterator last,
		       Bezier& b, points::iterator& split, real* maxError);
  vec2 leftTangent(points::const_iterator p);
  vec2 rightTangent(points::const_iterator p);
  vec2 centerTangent(points::const_iterator p);
  void chordLengthParameterize(points::iterator first, points::iterator last);
  void reparameterize(points::iterator first, points::iterator last,
		      Bezier& b);
  
  beziers bs;
  real length;
  real pixel_size;
  
public:
  Stroke();
  Stroke(Input& in, real error = 4.0);
  bool empty() const;
  void drawSpline() const;
  void drawCtrlPts() const;
  void drawTgts() const;
  void drawDerivatives() const;
  void drawCurvatureVector() const;
};

#endif // STROKE_H
