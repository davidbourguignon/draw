#ifndef POINT_H
#define POINT_H

#include "vec2.h"

template < class Real, class Vec = Vec2<Real> >
class Point {
public:
  Point();
  Point(const Vec& v, const Real p = 0.0);
  
  Vec pos; // Position
  Real u;  // Parameter value
};

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline Point<Real, Vec>::
Point()
  : pos(), u() {
}

template <class Real, class Vec>
inline Point<Real, Vec>::
Point(const Vec& v, const Real p)
  : pos(v), u(p) {
}

#endif // POINT_H
