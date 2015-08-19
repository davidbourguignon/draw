#ifndef BEZIER_BASE_H
#define BEZIER_BASE_H

#include <vector>
#include "vec2.h"
#include "vec3.h"

template <class Real, class Vec>
class Bezier_Base {
public:
  typedef Real                       real;
  typedef Vec                        vec;
  typedef Vec                        control_point;
  typedef std::vector<control_point> control_points;
  
  Bezier_Base();
  Bezier_Base(const typename control_points::size_type size);
  Bezier_Base(const Bezier_Base& b);
  void drawControlPoints() const;
  static Real Bernstein(const int i, const int n, const Real t);
  
  control_points V;
};

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline Bezier_Base<Real, Vec>::
Bezier_Base()
  : V() {}

template <class Real, class Vec>
inline Bezier_Base<Real, Vec>::
Bezier_Base(const typename control_points::size_type size)
  : V(size) {}

template <class Real, class Vec>
inline Bezier_Base<Real, Vec>::
Bezier_Base(const Bezier_Base& b)
  : V(b.V) {}

template <class Real, class Vec>
inline Real Bezier_Base<Real, Vec>::
Bernstein(const int i, const int n, const Real t) {
  const Real binomial_coefficient
    =  Numerics<int>::factorial(n) /
      (Numerics<int>::factorial(n - i)*Numerics<int>::factorial(i));
  return binomial_coefficient * Numerics<real>::power(t, i) *
                                Numerics<real>::power(1.0 - t, n - i);
}

template <class Real, class Vec>
inline void Bezier_Base<Real, Vec>::
drawControlPoints() const {}

template <>
inline void Bezier_Base< GLfloat, Vec3<GLfloat> >::
drawControlPoints() const {
  glBegin(GL_POINTS);
  for (control_points::const_iterator p = V.begin(); p != V.end(); p++) {
    glVertex3fv(&(*p)[0]);
  }
  glEnd();
}

template <>
inline void Bezier_Base< GLdouble, Vec3<GLdouble> >::
drawControlPoints() const {
  glBegin(GL_POINTS);
  for (control_points::const_iterator p = V.begin(); p != V.end(); p++) {
    glVertex3dv(&(*p)[0]);
  }
  glEnd();
}

#endif // BEZIER_BASE_H
