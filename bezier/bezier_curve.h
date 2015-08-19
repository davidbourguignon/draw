#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include <GL/gl.h>
#include <bezier_base.h>

template <class Real, class Vec>
class Bezier_Curve : public Bezier_Base<Real, Vec> {
private:
  typedef Bezier_Base<Real, Vec> base;
  
public:
  Bezier_Curve();
  Bezier_Curve(const int degree_u);
  Bezier_Curve(const Bezier_Curve& b);
  void evaluate(const Real u, Vec& Q) const;
  void evaluateDerivative(const int order, const Real u, Vec& Q) const;
  void draw(const GLint nstep_u) const;
};

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline Bezier_Curve<Real, Vec>::
Bezier_Curve()
  : Bezier_Base<Real, Vec>() {}

template <class Real, class Vec>
inline Bezier_Curve<Real, Vec>::
Bezier_Curve(const int degree_u)
  : Bezier_Base<Real, Vec>(degree_u + 1) {}

template <class Real, class Vec>
inline Bezier_Curve<Real, Vec>::
Bezier_Curve(const Bezier_Curve& b)
  : Bezier_Base<Real, Vec>(b) {}

template <class Real, class Vec>
inline void Bezier_Curve<Real, Vec>::
evaluate(const Real u, Vec& Q) const {
  typename base::control_points::size_type degree_u = V.size() - 1;
  typename base::control_points V_tmp(V);
  
  /* Triangle computation: de Casteljau's algorithm */
  for (int i = 1; i <= degree_u; i++) {
    typename base::control_points::iterator p = V_tmp.begin();
    for (; p != V_tmp.end() - i; p++) {
      (*p) = (*p)*(1.0 - u) + (*(p+1))*(u);
    }
  }
  
  Q = V_tmp.front(); /* Point on curve at parameter u */
}

template <class Real, class Vec>
inline void Bezier_Curve<Real, Vec>::
evaluateDerivative(const int order, const Real u, Vec& Q) const {
  typename base::control_points::size_type degree_u = V.size() - 1;
  typename base::control_points V_tmp(V);
  
  /* Nth order differentiation of the de Casteljau's algorithm */
  for (int j = 1; j <= order; j++) {
    typename base::control_points::iterator p = V_tmp.begin();
    for (; p != V_tmp.end() - j; p++) {
      (*p) = (*(p+1)) - (*p);
    }
  }
  for (int i = order + 1; i <= degree_u; i++) {
    typename base::control_points::iterator p = V_tmp.begin();
    for (; p != V_tmp.end() - i; p++) {
      (*p) = (*p)*(1.0 - u) + (*(p+1))*(u);
    }
  }
  
  /* Factorial */
  int n = 1;
  int m = 1;
  for (int j = 1; j <= degree_u; j++) {
    n *= j;
  }
  for (int i = 1; i <= degree_u - order; i++) {
    m *= i;
  }
  
  Q = V_tmp.front()*(n/m); /* Nth order derivative at parameter t */
}

template <class Real, class Vec>
inline void Bezier_Curve<Real, Vec>::
draw(const GLint nstep_u) const {}

template <>
inline void Bezier_Curve< GLfloat, Vec3<GLfloat> >::
draw(const GLint nstep_u) const {
  glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, V.size(), &V[0][0]);
  glMapGrid1f(nstep_u, 0.0, 1.0);
  glEvalMesh1(GL_LINE, 0, nstep_u);
}

template <>
inline void Bezier_Curve< GLdouble, Vec3<GLdouble> >::
draw(const GLint nstep_u) const {
  glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, V.size(), &V[0][0]);
  glMapGrid1d(nstep_u, 0.0, 1.0);
  glEvalMesh1(GL_LINE, 0, nstep_u);
}

#endif // BEZIER_CURVE_H
