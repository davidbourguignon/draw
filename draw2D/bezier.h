#ifndef BEZIER_H
#define BEZIER_H

#include <fstream>
#include <vector>
#include "vec3.h"
#include "point.h"

template < class Real, class Vec = Vec2<Real> >
class Bezier {
public:
  typedef Real             real;
  typedef Vec              vec;
  typedef std::vector<Vec> ctrl_points;
  
  Bezier();
  Bezier(const int degree);
  Bezier(const Bezier& b);
  
  /* Evaluate a Bezier curve at a particular parameter value */
  void evaluate(const Real t, Vec& Q) const;
  void evalDerivative(const int order, const Real t, Vec& Q) const;
  bool evalDiffGeomProperties(const Real t, Vec& Q, Vec& Q_prime,
			      Vec& Q_second, Vec& CC) const;
  
  /* Bernstein polynomials */
  static const Real B30(const Real u);
  static const Real B31(const Real u);
  static const Real B32(const Real u);
  static const Real B33(const Real u);
  
  ctrl_points V;  // Control points
};

template < class Real, class Vec = Vec2<Real> >
class Bezier_Augmented : public Bezier<Real, Vec> {
public:
  typedef std::vector<Real> parameters;
  typedef std::vector<Vec>  curv_centers;
  typedef std::vector<Real> radii;
  typedef std::vector<Vec>  normals;
  
  Bezier_Augmented();
  Bezier_Augmented(const int degree);
  Bezier_Augmented(const Bezier_Augmented& b);
  void evalLength(const int nstep);
  /*
   *  findNewtonRaphsonRoot :
   *  Use Newton-Raphson iteration to find better root.
   */
  void findNewtonRaphsonRoot(Point<Real, Vec>& P);
  
  void evalParameters();
  void computeRadii();
  void computeNormals();
  void read(std::ifstream& file_in);
  void write(std::ofstream& file_out) const;
  
  Real length;
  parameters T;   // Parameter value in [0;1]
  curv_centers C; // Center of curvature
  radii R;        // Radius of curvature
  normals N;      // Normal to the curve in the projection plane
                  // (for each control point)
};

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline Bezier<Real, Vec>::
Bezier()
  : V() {}

template <class Real, class Vec>
inline Bezier<Real, Vec>::
Bezier(const int degree)
  : V(degree+1) {}

template <class Real, class Vec>
inline Bezier<Real, Vec>::
Bezier(const Bezier& b)
  : V(b.V) {}

template <class Real, class Vec>
inline void Bezier<Real, Vec>::
evaluate(const Real t, Vec& Q) const {
  ctrl_points V_tmp(V);    /* Local copy of control points */
  int degree = V.size()-1; /* Degree of the Bezier curve */
  
  /* Triangle computation: de Casteljau's algorithm */
  for (int i = 1; i <= degree; i++) {
    for (typename ctrl_points::iterator p = V_tmp.begin();
	 p != V_tmp.end()-i; p++) {
      (*p) = (*p)*(1.0 - t) + (*(p+1))*(t);
    }
  }
  Q = V_tmp.front(); /* Point on curve at parameter t */
}

template <class Real, class Vec>
inline void Bezier<Real, Vec>::
evalDerivative(const int order, const Real t, Vec& Q) const {
  ctrl_points V_tmp(V);    /* Local copy of control points */
  int degree = V.size()-1; /* Degree of the Bezier curve */
  
  /* Nth order differentiation of the de Casteljau's algorithm */
  for (int j = 1; j <= order; j++) {
    for (typename ctrl_points::iterator p = V_tmp.begin();
	 p != V_tmp.end()-j; p++) {
      (*p) = (*(p+1)) - (*p);
    }
  }
  for (int i = order+1; i <= degree; i++) {
    for (typename ctrl_points::iterator p = V_tmp.begin();
	 p != V_tmp.end()-i; p++) {
      (*p) = (1.0 - t)*(*p) + (t)*(*(p+1));
    }
  }
  
  /* Factorial */
  int n = 1;
  int m = 1;
  for (int j = 1; j <= degree; j++) {
    n *= j;
  }
  for (int i = 1; i <= degree - order; i++) {
    m *= i;
  }
  
  Q = V_tmp.front()*(n/m); /* Nth order derivative at parameter t */
}

template <class Real, class Vec>
inline bool Bezier<Real, Vec>::
evalDiffGeomProperties(const Real t, Vec& Q, Vec& Q_prime, Vec& Q_second,
		       Vec& CC) const {
  evaluate(t, Q);
  evalDerivative(1, t, Q_prime);
  evalDerivative(2, t, Q_second);
  Real tmp = dot(Q_prime, Q_prime)/cross(Q_prime, Q_second);
  if (Numerics<Real>::isfinite(tmp)) {
    CC.setx(Q.x() - Q_prime.y()*tmp); // Center of curvature
    CC.sety(Q.y() + Q_prime.x()*tmp);
    return true;
  }
  else {
    CC.setx(Q.x() - Q_prime.y());
    CC.sety(Q.y() + Q_prime.x());
    return false;
  }
}

template <class Real, class Vec>
inline const Real Bezier<Real, Vec>::
B30(const Real u) {
  Real tmp = 1.0 - u;
  return (tmp * tmp * tmp);
}

template <class Real, class Vec>
inline const Real Bezier<Real, Vec>::
B31(const Real u) {
  Real tmp = 1.0 - u;
  return (3.0 * u * tmp * tmp);
}

template <class Real, class Vec>
inline const Real Bezier<Real, Vec>::
B32(const Real u) {
  Real tmp = 1.0 - u;
  return (3.0 * u * u * tmp);
}

template <class Real, class Vec>
inline const Real Bezier<Real, Vec>::
B33(const Real u) {
  return (u * u * u);
}

template <class Real, class Vec>
inline Bezier_Augmented<Real, Vec>::
Bezier_Augmented()
  : Bezier<Real, Vec>(),
  length(0.0), T(), C(), R(), N() {}

template <class Real, class Vec>
inline Bezier_Augmented<Real, Vec>::
Bezier_Augmented(const int degree)
  : Bezier<Real, Vec>(degree),
  length(0.0), T(degree+1), C(degree+1), R(degree+1), N(degree+1) {}

template <class Real, class Vec>
inline Bezier_Augmented<Real, Vec>::
Bezier_Augmented(const Bezier_Augmented& b)
  : Bezier<Real, Vec>(b),
  length(b.length), T(b.T), C(b.C), R(b.R), N(b.N) {}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
findNewtonRaphsonRoot(Point<Real, Vec>& P) {
  /* Q, Q' and Q'' evaluated at u */
  Vec Q_u, Q1_u, Q2_u;
  evaluate(P.u, Q_u);
  evalDerivative(1, P.u, Q1_u);
  evalDerivative(2, P.u, Q2_u);
  
  /* Compute f(u)/f'(u) */
  Real numerator = (Q_u.x() - P.pos.x()) * (Q1_u.x()) +
                   (Q_u.y() - P.pos.y()) * (Q1_u.y());
  Real denominator = (Q1_u.x()) * (Q1_u.x()) +
                     (Q1_u.y()) * (Q1_u.y()) +
		     (Q_u.x() - P.pos.x()) * (Q2_u.x()) +
                     (Q_u.y() - P.pos.y()) * (Q2_u.y());
  
  /* u = u - f(u)/f'(u) */
  P.u -= numerator/denominator;
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
evalLength(const int nstep) {
  const Real step_size = 1.0/nstep;
  Vec Q_prev, Q_curr;
  evaluate(0.0, Q_prev);
  for (int i = 1; i <= nstep; i++) { // Magic number!
    evaluate(i*step_size, Q_curr);
    length += dist(Q_prev, Q_curr);
    Q_prev = Q_curr;
  }
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
evalParameters() {
  /* Warning: Only valid for cubic Bezier curves! */
#if 0
  Vec l = V[3] - V[0];
  Real lsqn_inv = 1.0/l.sqnorm();
  const Real default_value = 1.0/3.0;
  const Real d1 = dot(l, V[1] - V[0]);
  Real u1;
  if (d1 < 0.0) {
    u1 = default_value;
  }
  else {
    u1 = d1*lsqn_inv;
  }
  //cerr << "Param u1 = " << u1 << endl;
  const Real d2 = dot(-l, V[2] - V[3]);
  Real u2;
  if (d2 < 0.0) {
    u2 = 1.0 - default_value;
  }
  else {
    u2 = 1.0 - d2*lsqn_inv;
  }
  //cerr << "Param u2 = " << u2 << endl;
#else
  const Real default_value = 1.0/3.0;
  const Real u1 = default_value;
  const Real u2 = 1.0 - default_value;
#endif
  Point<Real, Vec> P1(V[1], u1);
  Point<Real, Vec> P2(V[2], u2);
  
  Vec Q, Q_prime;
  const int maxIterations = 4;   // Magic number!
  const Real threshold = 1.0e-3; // Magic number!
  int iter;
  for (iter = 0; iter < maxIterations; iter++) {
    findNewtonRaphsonRoot(P1);
    //cerr << "P1.u " << P1.u << endl;
    evaluate(P1.u, Q);
    evalDerivative(1, P1.u, Q_prime);
    Real result = Numerics<Real>::fpabs(dot(Q - V[1], Q_prime));
    if (result < threshold) {
      break;
    }
  }
  for (iter = 0; iter < maxIterations; iter++) {
    findNewtonRaphsonRoot(P2);
    //cerr << "P2.u " << P2.u << endl;
    evaluate(P2.u, Q);
    evalDerivative(1, P2.u, Q_prime);
    Real result = Numerics<Real>::fpabs(dot(Q - V[2], Q_prime));
    if (result < threshold) {
      break;
    }
  }
  
  if (T.empty()) {
    T.push_back(0.0);
    T.push_back(P1.u);
    T.push_back(P2.u);
    T.push_back(1.0);
  }
  else {
    T[0] = 0.0;
    T[1] = P1.u;
    T[2] = P2.u;
    T[3] = 1.0;
  }
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
computeRadii() {
  if (R.empty()) {
    for (int i = 0; i < V.size(); i++) {
      R.push_back((V[i] - C[i]).norm());
    }
  }
  else {
    for (int i = 0; i < V.size(); i++) {
      R[i] = (V[i] - C[i]).norm();
    }
  }
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
computeNormals() {
  if (N.empty()) {
    for (int i = 0; i < V.size(); i++) {
      N.push_back((V[i] - C[i]).normalize());
    }
  }
  else {
    for (int i = 0; i < V.size(); i++) {
      N[i] = (V[i] - C[i]).normalize();
    }
  }
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
read(std::ifstream& file_in) {}

template <>
inline void Bezier_Augmented< double, Vec3<double> >::
read(std::ifstream& file_in) {
  char line[256];
  file_in.getline(line, 256, '\n');
  int n;
  sscanf(line, "%d", &n);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf", &length);
  
  double r;
  Vec3<double> v;
  
  int i;
  for (i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf %lf %lf", &v[0], &v[1], &v[2]);
    V.push_back(v);
  }
  for (i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf", &r);
    T.push_back(r);
  }
  for (i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf %lf %lf", &v[0], &v[1], &v[2]);
    C.push_back(v);
  }
  for (i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf", &r);
    R.push_back(r);
  }
  for (i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf %lf %lf", &v[0], &v[1], &v[2]);
    N.push_back(v);
  }
}

template <class Real, class Vec>
inline void Bezier_Augmented<Real, Vec>::
write(std::ofstream& file_out) const {
  file_out << V.size() << endl;
  file_out << length << endl;
  
  for (typename ctrl_points::const_iterator cp = V.begin();
       cp != V.end(); cp++) {
    file_out << (*cp) << endl;
  }
  for (typename parameters::const_iterator tp = T.begin();
       tp != T.end(); tp++) {
    file_out << (*tp) << endl;
  }
  for (typename curv_centers::const_iterator ccp = C.begin();
       ccp != C.end(); ccp++) {
    file_out << (*ccp) << endl;
  }
  for (typename radii::const_iterator rp = R.begin();
       rp != R.end(); rp++) {
    file_out << (*rp) << endl;
  }
  for (typename normals::const_iterator np = N.begin();
       np != N.end(); np++) {
    file_out << (*np) << endl;
  }
}

#endif // BEZIER_H
