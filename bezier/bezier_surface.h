#ifndef BEZIER_SURFACE_H
#define BEZIER_SURFACE_H

#include <GL/gl.h>
#include <bezier_base.h>

template <class Real, class Vec>
class Bezier_Surface : public Bezier_Base<Real, Vec> {
private:
  typedef Bezier_Base<Real, Vec> base;
  
public:
  Bezier_Surface();
  Bezier_Surface(const int degree_u, const int degree_v);
  Bezier_Surface(const Bezier_Surface& b);
  void setControlPoint(const int i, const int j, const Vec& V_ij);
  void evaluate(const Real u, const Real v, Vec& Q) const;
#if 0
  void evaluateDerivative(const int order,
		          const Real u, const Real v, Vec& Q) const;
#endif
  void draw(const GLint nstep_u, const GLint nstep_v) const;
  
  typename base::control_points::size_type order_u, order_v;
};

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline Bezier_Surface<Real, Vec>::
Bezier_Surface()
  : Bezier_Base<Real, Vec>(), order_u(0), order_v(0) {}

template <class Real, class Vec>
inline Bezier_Surface<Real, Vec>::
Bezier_Surface(const int degree_u, const int degree_v)
  : Bezier_Base<Real, Vec>((degree_u + 1)*(degree_v + 1)),
  order_u(degree_u + 1), order_v(degree_v + 1) {}

template <class Real, class Vec>
inline Bezier_Surface<Real, Vec>::
Bezier_Surface(const Bezier_Surface& b)
  : Bezier_Base<Real, Vec>(b), order_u(b.order_u), order_v(b.order_v) {}

template <class Real, class Vec>
inline void Bezier_Surface<Real, Vec>::
setControlPoint(const int i, const int j, const Vec& V_ij) {
  V[j*order_u + i] = V_ij;
}

template <class Real, class Vec>
inline void Bezier_Surface<Real, Vec>::
evaluate(const Real u, const Real v, Vec& Q) const {
  typename base::control_points::size_type degree_u = order_u - 1;
  typename base::control_points::size_type degree_v = order_v - 1;
  
  /* Tensor product approach */
  typename base::control_points V_tmp_v;
  for (int j = 0; j < order_v; j++) {
    typename base::control_points V_tmp_u(V.begin() + j*order_u,
				          V.begin() + (j+1)*order_u);
    // Triangle computation: de Casteljau's algorithm (along u)
    for (int i = 1; i <= degree_u; i++) {
      for (typename base::control_points::iterator p = V_tmp_u.begin();
	   p != V_tmp_u.end() - i; p++) {
        (*p) = (*p)*(1.0 - u) + (*(p+1))*(u);
      }
    }
    V_tmp_v.push_back(V_tmp_u.front());
  }
  // Triangle computation: de Casteljau's algorithm (along v)
  for (int j = 1; j <= degree_v; j++) {
    for (typename base::control_points::iterator p = V_tmp_v.begin();
	 p != V_tmp_v.end() - j; p++) {
      (*p) = (*p)*(1.0 - v) + (*(p+1))*(v);
    }
  }
  
  Q = V_tmp_v.front(); // Point on curve at parameters (u,v)
}

#if 0
template <class Real, class Vec>
inline void Bezier_Surface<Real, Vec>::
evaluateDerivative(const int order, const Real u, const Real v, Vec& Q) const {
}
#endif

template <class Real, class Vec>
inline void Bezier_Surface<Real, Vec>::
draw(const GLint nstep_u, const GLint nstep_v) const {}

template <>
inline void Bezier_Surface< GLfloat, Vec3<GLfloat> >::
draw(const GLint nstep_u, const GLint nstep_v) const {
  glMap2f(GL_MAP2_VERTEX_3, 0.0, 1.0,         3, order_u,
                            0.0, 1.0, order_u*3, order_v, &V[0][0]);
  glMapGrid2f(nstep_u, 0.0, 1.0, nstep_v, 0.0, 1.0);
  glEvalMesh2(GL_FILL, 0, nstep_u, 0, nstep_v);
}

template <>
inline void Bezier_Surface< GLdouble, Vec3<GLdouble> >::
draw(const GLint nstep_u, const GLint nstep_v) const {
  glMap2d(GL_MAP2_VERTEX_3, 0.0, 1.0,         3, order_u,
                            0.0, 1.0, order_u*3, order_v, &V[0][0]);
  glMapGrid2d(nstep_u, 0.0, 1.0, nstep_v, 0.0, 1.0);
  glEvalMesh2(GL_FILL, 0, nstep_u, 0, nstep_v);
}

#endif // BEZIER_SURFACE_H
