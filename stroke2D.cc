#include "stroke2D.h"

using namespace std;

/*
 *  IMPORTANT WARNING! IMPORTANT WARNING! IMPORTANT WARNING!
 *  All these functions use iterators on points... It works because points
 *  array is never modified... Beware! Better use array indices rather than
 *  iterators!
 */

/*
 *  fitCurve :
 *  Fit a Bezier curve to a set of digitized points 
 */
void Stroke2D::fitCurve(const points::iterator first,
			const points::iterator last, real error) {
  /* Number of points in subset */
  points::difference_type npoints = std::distance(first, last) + 1;
  
  /* Unit tangent vectors at endpoints */
  vec2 tanv1(leftTangent(first, npoints));
  vec2 tanv2(rightTangent(last, npoints));
  fitCubic(first, last, tanv1, tanv2, error);
}

/*
 *  fitCubic :
 *  Fit a Bezier curve to a (sub)set of digitized points
 */
void Stroke2D::fitCubic(const points::iterator first,
			const points::iterator last,
			const vec2& tanv1, const vec2& tanv2, real error) {
  bezier b(3); /* Fitted cubic Bezier curve */
  
  /* Number of points in subset */
  points::difference_type npoints = std::distance(first, last) + 1;
  
  /*  Use heuristic if region only has two points in it */
  if (npoints == 2) {
    real d = dist((*last).pos, (*first).pos)/3.0;
    b.V[0] = (*(first)).pos;
    b.V[3] = (*(last)).pos;
    b.V[1] = b.V[0] + tanv1*d;
    b.V[2] = b.V[3] + tanv2*d;
    bs.push_back(b);
    return;
  }
  
  /* Parameterize points, and attempt to fit curve */
  chordLengthParameterize(first, last);
  generateBezier(first, last, tanv1, tanv2, b);
  
  /*  Find max deviation of points to fitted curve */
  real maxError;          /* Maximum fitting error */
  points::iterator split; /* Point to split point set at */
  computeMaxError(first, last, b, split, &maxError);
  if (maxError < error) {
    bs.push_back(b);
    return;
  }
  
  /* If error not too large, try some reparameterization */
  /* and iteration */
  real iterationError = error*error; /* Error below which you try iterating */
  int maxIterations = 4;             /* Max times to try iterating */
  if (maxError < iterationError) {
    for (int i = 0; i < maxIterations; i++) {
      reparameterize(first, last, b);
      generateBezier(first, last, tanv1, tanv2, b);
      computeMaxError(first, last, b, split, &maxError);
      if (maxError < error) {
	bs.push_back(b);
	return;
      }
    }
  }
  
  /* Fitting failed -- split at max error point and fit recursively */
  vec2 tanv_center(centerTangent(split)); /* Unit tangent vector */
                                          /* at split point */
  fitCubic(first, split, tanv1, tanv_center, error);
  fitCubic(split, last, -tanv_center, tanv2, error);
}

class A {
  typedef GLdouble   real;
  typedef Vec2<real> vec2;
  
public:
  A() {}
  A(const vec2& v1, const vec2& v2)
    : A1(v1), A2(v2) {}
  
  vec2 A1, A2;
};

/*
 *  generateBezier :
 *  Use least-squares method to find Bezier control points for region.
 */
void Stroke2D::generateBezier(const points::const_iterator first,
			      const points::const_iterator last,
			      const vec2& tanv1, const vec2& tanv2,
			      bezier& b) {
  std::vector<A> As; /* Precomputed rhs for eqn */
  
  /* Compute the A's */
  points::const_iterator p;
  for (p = first; p <= last; p++) {
    real u = (*p).u;
    As.push_back(A(tanv1*bezier::B31(u),tanv2*bezier::B32(u)));
  }
  
  /* Create the C and X matrices */
  real C[2][2];
  real X[2];
  
  C[0][0] = 0.0; C[0][1] = 0.0;
  C[1][0] = 0.0; C[1][1] = 0.0;
  
  X[0] = 0.0;
  X[1] = 0.0;
  
  std::vector<A>::iterator a = As.begin();
  for (p = first; p <= last; p++, a++) {
    C[0][0] += dot((*a).A1,(*a).A1);
    C[0][1] += dot((*a).A1,(*a).A2);
    C[1][0] = C[0][1];
    C[1][1] += dot((*a).A2, (*a).A2);
    
    real u = (*p).u;
    vec2 tmp = (*p).pos
               - ((*first).pos*bezier::B30(u) + (*first).pos*bezier::B31(u) +
		   (*last).pos*bezier::B32(u) + (*last).pos*bezier::B33(u));
    
    X[0] += dot((*a).A1,tmp);
    X[1] += dot((*a).A2,tmp);
  }
  
  /* Compute the determinants of C and X */
  real det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
  det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];
  
  /* Finally, derive alpha values */
  
  /* Old solution: doesn't work because problems appear way before */
  /* determinant equals zero. */
  /*if (det_C0_C1 == 0.0) {
    det_C0_C1 = (C[0][0] * C[1][1]) * 1.0e-12;
  }*/
  
  /* New solution to handle near zero determinants. */
  if (det_C0_C1 < 1.0e-3) {
#if DEBUG
    cerr << "Warning: Near zero determinant! Det = " << det_C0_C1 << endl;
#endif
    real d = dist((*last).pos, (*first).pos)/3.0;
    b.V[0] = (*(first)).pos;
    b.V[3] = (*(last)).pos;
    b.V[1] = b.V[0] + tanv1*d;
    b.V[2] = b.V[3] + tanv2*d;
    return;
  }
  real alpha_l, alpha_r; /* Alpha values, left and right */
  alpha_l = det_X_C1 / det_C0_C1;
  alpha_r = det_C0_X / det_C0_C1;
  
  /* If alpha negative, use the Schmitt et al. heuristic (see text) */
  /* (if alpha is 0, you get coincident control points that lead to */
  /* divide by zero in any subsequent findNewtonRaphsonRoot() call. */
  if (alpha_l < 1.0e-6 || alpha_r < 1.0e-6) {
    real d = dist((*last).pos, (*first).pos)/3.0;
    b.V[0] = (*(first)).pos;
    b.V[3] = (*(last)).pos;
    b.V[1] = b.V[0] + tanv1*d;
    b.V[2] = b.V[3] + tanv2*d;
    return;
  }
  
  /* First and last control points of the Bezier curve are */
  /* positioned exactly at the first and last data points. */
  /* Control points 1 and 2 are positioned an alpha distance out */
  /* on the tangent vectors, left and right, respectively. */
  b.V[0] = (*(first)).pos;
  b.V[3] = (*(last)).pos;
  b.V[1] = b.V[0] + tanv1*alpha_l;
  b.V[2] = b.V[3] + tanv2*alpha_r;
}

/*
 *  findNewtonRaphsonRoot :
 *  Use Newton-Raphson iteration to find better root.
 */
void Stroke2D::findNewtonRaphsonRoot(bezier& Q, point& P) {
  vec2 Q_u, Q1_u, Q2_u; /* Q, Q' and Q'' evaluated at u */
  
  /* Compute Q(u) */
  Q.evaluate(P.u, Q_u);
  
  /* Generate control vertices for Q' */
  bezier Q1(2);
  bezier::ctrl_points::iterator p1 = Q1.V.begin();
  bezier::ctrl_points::iterator p  = Q.V.begin();
  for (; p1 != Q1.V.end(); p1++, p++) {
    (*p1) = ((*(p+1)) - (*p)) * 3.0;
  }
  
  /* Generate control vertices for Q'' */
  bezier Q2(1);
  bezier::ctrl_points::iterator p2 = Q2.V.begin();
  p1 = Q1.V.begin();
  for (; p2 != Q2.V.end(); p2++, p1++) {
    (*p2) = ((*(p1+1)) - (*p1)) * 2.0;
  }
  
  /* Compute Q'(u) and Q''(u) */
  Q1.evaluate(P.u, Q1_u);
  Q2.evaluate(P.u, Q2_u);
  
  /* Compute f(u)/f'(u) */
  real numerator = (Q_u.x() - P.pos.x()) * (Q1_u.x()) +
                   (Q_u.y() - P.pos.y()) * (Q1_u.y());
  real denominator = (Q1_u.x()) * (Q1_u.x()) +
                     (Q1_u.y()) * (Q1_u.y()) +
		     (Q_u.x() - P.pos.x()) * (Q2_u.x()) +
                     (Q_u.y() - P.pos.y()) * (Q2_u.y());
  
  /* u = u - f(u)/f'(u) */
  P.u -= numerator/denominator;
}

/*
 *  computeMaxError :
 *  Find the maximum squared distance of digitized points
 *  to fitted curve.
 */
void Stroke2D::computeMaxError(const points::iterator first,
			       const points::iterator last, bezier& b,
			       points::iterator& split, real* maxError) {
  points::difference_type splitPoint
    = static_cast<points::difference_type>(0.5*(std::distance(first, last)+1));
  split = first + splitPoint; // Added!
  real maxDistance = 0.0; /* Maximum distance */
  for (points::iterator p = first+1; p != last; p++) {
    vec2 P; /* Point on curve */
    b.evaluate((*p).u, P);
    real distance = dist(P, (*p).pos); /* Current error */
    if (distance > maxDistance) {
      maxDistance = distance;
      split = p;
    }
  }
  *maxError = maxDistance;
}

/*
 *  leftTangent, rightTangent, centerTangent :
 *  Approximate unit tangents at endpoints and "center" of digitized curve
 *  by fitting a least-square line to the points in the neighborhood of the end
 *  points or by averaging vectors from the endpoints to the next n points.
 */
Stroke2D::vec2 Stroke2D::leftTangent(const points::const_iterator p,
				     const points::difference_type npoints) {
  if (npoints < 10) { // 5 is the minimum!
    return ((*(p+1)).pos - (*p).pos).normalize();
  }
  else {
    return (0.5*((*(p+2)).pos + (*(p+1)).pos - 2.0*(*p).pos)).normalize();
  }
}
Stroke2D::vec2 Stroke2D::rightTangent(const points::const_iterator p,
				      const points::difference_type npoints) {
  if (npoints < 10) { // 5 is the minimum!
    return ((*(p-1)).pos - (*p).pos).normalize();
  }
  else {
    return (0.5*((*(p-2)).pos + (*(p-1)).pos - 2.0*(*p).pos)).normalize();
  }
}
Stroke2D::vec2 Stroke2D::centerTangent(const points::const_iterator p) {
  return ((*(p-1)).pos - (*(p+1)).pos).normalize();
}

/*
 *  chordLengthParameterize :
 *  Assign parameter values to digitized points 
 *  using relative distances between points.
 */
void Stroke2D::chordLengthParameterize(const points::iterator first,
				       const points::iterator last) {
  (*first).u = 0.0;
  points::iterator p;
  for (p = first+1; p <= last; p++) {
    (*p).u = (*(p-1)).u + dist((*p).pos, (*(p-1)).pos);
  }
  real u_last = (*last).u;
  for (p = first+1; p != last; p++) {
    (*p).u /= u_last;
  }
  (*last).u = 1.0;
}

/*
 *  reparameterize :
 *  Given set of points and their parameterization, try to find
 *  a better parameterization.
 */
void Stroke2D::reparameterize(const points::iterator first,
			      const points::iterator last, bezier& b) {
  for (points::iterator p = first; p <= last; p++) {
    findNewtonRaphsonRoot(b, (*p));
  }
}

void Stroke2D::fit(Input& in, const real error) {
  /* Filter */
  /*if (in.positions.size() > 2)
    in.fair(); // Useful?*/
  
  /* Locate corners */
  points::iterator first = in.positions.begin();
  points::iterator last  = in.positions.end()-1;
  points::iterator p;
  points::iterator split = in.positions.begin()+1;
  bool have_corner = false;
  for (p = first+1; p != last; p++) {
    real cos_a = cosAng((*(p+1)).pos - (*p).pos, (*(p-1)).pos - (*p).pos);
    if (cos_a > 0.0) {
      have_corner = true;
      split = p;
      fitCurve(first, split, error);
      first = split;
    }
  }
  if (have_corner) {
    fitCurve(split, last, error);
  }
  else {
    fitCurve(in.positions.begin(), in.positions.end()-1, error);
  }
}

void Stroke2D::evalLength() {
  beziers::iterator b;
  
  /* First pass */
  const int nstep_rough = 3; // Magic number!
  for (b = bs.begin(); b != bs.end(); b++) {
    (*b).evalLength(nstep_rough);
    length += (*b).length;
  }
  // I'm not sure this first pass is really useful but...
  
  /* Second pass */
  const real length_rough = length;
  const real length_rough_inv = 1.0/length_rough;
  length = 0.0;
  int nstep_tot = static_cast<int>(steps_per_unit_length*length_rough);
  if (nstep_tot < 3) {
    nstep_tot = 3;
  }
  for (b = bs.begin(); b != bs.end(); b++) {
    int nstep = static_cast<int>(((*b).length*length_rough_inv)*nstep_tot);
    if (nstep == 0) {
      nstep = 1;
    }
    (*b).length = 0.0;
    (*b).evalLength(nstep);
    length += (*b).length;
  }
  
  /* Third pass */
  const real length_inv = 1.0/length;
  for (b = bs.begin(); b != bs.end(); b++) {
    relative_lengths.push_back((*b).length*length_inv);
  }
}

class Curve_Properties {
  typedef Stroke2D::real real;
  typedef Stroke2D::vec2 vec2;
  typedef Stroke2D::beziers::iterator iterb;
  
public:
  Curve_Properties() {}
  Curve_Properties(const iterb& b, const real& t) {
    bool is_number
      = (*b).evalDiffGeomProperties(t, Q, Q_prime, Q_second, CC);
    CV = CC - Q;
    R = CV.norm();
    CV /= R;
    if (is_number) {
      if (R > 1.0e+3) { // Magic number!
	is_finite = false;
      }
      else {
	is_finite = true;
      }
    }
    else {
      is_finite = false;
    }
  }
  Curve_Properties(const vec2& q_p, const vec2& cv)
    : Q_prime(q_p), CV(cv) {}
  
  vec2 Q, Q_prime, Q_second; // Position, first and second derivative
  vec2 CC, CV;               // Center of curvature, curvature vector
  real R;                    // Radius
  bool is_finite;
};

class Inflection_Point {
  typedef Stroke2D::real                     real;
  typedef Stroke2D::vec2                     vec2;
  typedef Stroke2D::beziers::difference_type diff;
  
public:
  Inflection_Point() {}
  Inflection_Point(diff index, real param, vec2 p)
    : n(index), t(param), pos(p) {}
  
  diff n;
  real t;
  vec2 pos;
};

bool Stroke2D::testLength(const beziers::difference_type nb_first,
			  const real t_first,
			  const beziers::difference_type nb_last,
			  const real t_last) {
  const real min_dist_rel = 0.125; // Magic number!
  
  bool is_not_enough = true;
  beziers::difference_type n;
  real dist_rel;
  
  if (nb_first == -1) { // if nb_first = -1 we get our first case
    n = 0;
    dist_rel = 0.0;
  }
  else {
    if (t_first == 0.0) {
      n = nb_first;
      dist_rel = 0.0;
    }
    else {
      n = nb_first + 1;
      dist_rel = (1.0 - t_first)*relative_lengths[nb_first];
    }
  }
  while (is_not_enough && n < nb_last) {
    dist_rel += relative_lengths[n];
    if (dist_rel > min_dist_rel) {
      is_not_enough = false;
      return !is_not_enough;
    }
    else {
      n++;
    }
  }
  if (is_not_enough && n == nb_last) {
    dist_rel += (t_last)*relative_lengths[n];
  }
  if (dist_rel > min_dist_rel) {
    is_not_enough = false;
  }
  return !is_not_enough; // "true" means test succeeded.
}

void minmax(const Stroke2D::vec2& value,
	    Stroke2D::vec2& min, Stroke2D::vec2& max) {
  Stroke2D::real value_x = value.x();
  Stroke2D::real value_y = value.y();
  
  if (value_x < min.x()) {
    min.setx(value_x);
  }
  else if (value_x > max.x()) {
    max.setx(value_x);
  }
  if (value_y < min.y()) {
    min.sety(value_y);
  }
  else if (value_y > max.y()) {
    max.sety(value_y);
  }
}

void Stroke2D::getBoundingBox(const vec2& pt_first,
			      const beziers::difference_type nb_first,
		              const real t_first,
		              const vec2& pt_last,
			      const beziers::difference_type nb_last,
		              const real t_last, vec2& min, vec2& max) {
  beziers::difference_type n;
  
  min = max = pt_first;
  if (nb_first == -1) { // See testLength() method
    n = 0;
  }
  else {
    if (t_first == 0.0) {
      n = nb_first;
    }
    else {
      n = nb_first + 1;
      int index_first = bs[nb_first].T.size() - 2;
      while (t_first < bs[nb_first].T[index_first]) {
        minmax(bs[nb_first].V[index_first], min, max);
        index_first--;
      }
    }
  }
  while (n < nb_last) {
    for (bezier::ctrl_points::const_iterator p = bs[n].V.begin();
	 p != bs[n].V.end() - 1; p++) {
      minmax((*p), min, max);
    }
    n++;
  }
  minmax(pt_last, min, max);
  if (n == nb_last) {
    int index_last = 0;
    while(t_last > bs[nb_last].T[index_last]) {
      minmax(bs[nb_last].V[index_last], min, max);
      index_last++;
    }
  }
}

Stroke2D::real radiusMax(const Stroke2D::vec2& min,
			 const Stroke2D::vec2& max) {
  const Stroke2D::real ratio = 0.4; /*1.0/3.0*/ // Magic number!
  
  Stroke2D::real radius;
  Stroke2D::real rx = max.x() - min.x();
  Stroke2D::real ry = max.y() - min.y();
  if (rx > ry) {
    radius = ratio*rx;
  }
  else {
    radius = ratio*ry;
  }
  return radius;
}

void correctCurvatureCenters(std::vector<Curve_Properties>& CPs,
  const std::vector<Curve_Properties>::difference_type n_first,
  const std::vector<Curve_Properties>::difference_type n_last,
  const Stroke2D::real radius_max, const bool first_box) {
  
  Stroke2D::real cCP;
  if (first_box) {
    cCP = cross(CPs[n_last].Q_prime, CPs[n_last].CV);
  }
  else {
    cCP = cross(CPs[n_first - 1].Q_prime, CPs[n_first - 1].CV);
  }
  std::vector<Curve_Properties>::difference_type n;
  for (n = n_first; n <= n_last; n++) {
    if (cross(CPs[n].Q_prime, CPs[n].CV)*cCP < 0.0) {
      if (!CPs[n].is_finite || CPs[n].R > radius_max) {
	CPs[n].R = radius_max;
      }
      CPs[n].CV = -CPs[n].CV;
      CPs[n].CC = CPs[n].Q + CPs[n].R*CPs[n].CV;
    }
    else {
      if (!CPs[n].is_finite || CPs[n].R > radius_max) {
	CPs[n].R = radius_max;
	CPs[n].CC = CPs[n].Q + radius_max*CPs[n].CV;
      }
    }
  }
}

void finalizeCurvatureCenters(Stroke2D::beziers& bs,
			      std::vector<Curve_Properties>& CPs) {
  typedef Stroke2D::real real;
  std::vector<Curve_Properties>::iterator cp;
  
  /* Compute mean */
  real mu = 0.0;
  for (cp = CPs.begin(); cp != CPs.end(); cp++) {
    mu += (*cp).R;
  }
  mu /= CPs.size();
  
  /* Scale */
  const real factor = 0.25; /*0.5*/ // Magic number!
  for (cp = CPs.begin(); cp != CPs.end(); cp++) {
    const real diff = mu - (*cp).R;
    (*cp).R = mu - factor*diff;
  }
  
  /* Copy data */
  cp = CPs.begin();
  for (Stroke2D::beziers::iterator b = bs.begin(); b != bs.end(); b++) {
    int i = 0;
    for (; i < (*b).C.size() - 1; i++, cp++) {
      (*b).C[i] = (*cp).Q + (*cp).R*(*cp).CV;
    }
    (*b).C[i] = (*cp).Q + (*cp).R*(*cp).CV;
  }
}

void Stroke2D::evalCurvatureCenters() {
  std::vector<Curve_Properties> CPs;
  std::vector<Curve_Properties>::difference_type n_first = 0;
  std::vector<Curve_Properties>::difference_type n_last = 0;
  
  // "First" curve properties and inflection point
  Curve_Properties CP2_prev(vec2::null(), vec2::null());
  Inflection_Point IP_prev(-1, 0.0, bs[0].V[0]);
  
  bool first_box = true;
  const beziers::iterator b_first = bs.begin();
  const beziers::iterator b_last = bs.end() - 1;
  const beziers::difference_type n_tot = std::distance(b_first, b_last);
  for (beziers::iterator b = bs.begin(); b != bs.end(); b++) {
    // Evaluate control points parameter values
    (*b).evalParameters();
    
    Curve_Properties CP0(b, (*b).T[0]);
    CPs.push_back(CP0);
    Curve_Properties CP1(b, (*b).T[1]);
    
    // Inflection point between two cubic Bezier curve
    if (cross(CP1.Q_prime, CP1.CV)*
	cross(CP2_prev.Q_prime, CP2_prev.CV) < 0.0) {
      assert(b != b_first);
      beziers::difference_type n = std::distance(b_first, b);
      if (testLength(IP_prev.n, IP_prev.t, n, 0.0) &&
	  testLength(n, 0.0, n_tot, 1.0)) {
	vec2 min, max;
	getBoundingBox(IP_prev.pos, IP_prev.n, IP_prev.t,
		       (*b).V[0], n, 0.0, min, max);
	n_last = CPs.size() - 2;
	correctCurvatureCenters(CPs, n_first, n_last, radiusMax(min, max),
				first_box);
	if (first_box) {
	  first_box = false;
	}
#if TEST_STROKE2D
	glColor3f(0.0, 0.0, 0.0); // Be careful!
	glBegin(GL_LINE_LOOP);
          glVertex2d(min.x(), min.y());
          glVertex2d(max.x(), min.y());
          glVertex2d(max.x(), max.y());
          glVertex2d(min.x(), max.y());
        glEnd();
#endif
	n_first = n_last + 1;
	IP_prev = Inflection_Point(n, 0.0, (*b).V[0]);
      }
    }
    
    CPs.push_back(CP1);
    Curve_Properties CP2(b, (*b).T[2]);
    CPs.push_back(CP2);
    Curve_Properties CP3(b, (*b).T[3]);
    
    // Inflection point in the middle of a cubic Bezier curve
    if (cross(CP0.Q_prime, CP0.CV)*cross(CP3.Q_prime, CP3.CV) < 0.0) {
      vec2 V0 = (*b).V[0];
      vec2 V1 = (*b).V[1];
      vec2 V2 = (*b).V[2];
      vec2 V3 = (*b).V[3];
      vec2 tmp1 = 2.0*V1 - V0 - V2;
      vec2 tmp2 = 3.0*V1 - V0 - 3.0*V2 + V3;
      real tx = tmp1.x()/tmp2.x();
      real ty = tmp1.y()/tmp2.y();
      real t = 0.5*(tx + ty);
      
      const real epsilon = 1.0e-3; // Magic number!
      if (t >= 0.0 - epsilon && t <= 1.0 + epsilon) {
	beziers::difference_type n = std::distance(b_first, b);
        if (testLength(IP_prev.n, IP_prev.t, n, t) &&
	    testLength(n, t, n_tot, 1.0)) {
	  vec2 inflection_pt;
	  (*b).evaluate(t, inflection_pt);
	  vec2 min, max;
	  getBoundingBox(IP_prev.pos, IP_prev.n, IP_prev.t,
		         inflection_pt, n, t, min, max);
	  int index = 0;
	  if (t < (*b).T[1]) {
	    index = 1;
	  }
	  n_last = CPs.size() - 2 - index;
	  correctCurvatureCenters(CPs, n_first, n_last, radiusMax(min, max),
				  first_box);
	  if (first_box) {
	    first_box = false;
	  }
#if TEST_STROKE2D
	  glColor3f(0.0, 0.0, 0.0); // Be careful!
	  glBegin(GL_LINE_LOOP);
            glVertex2d(min.x(), min.y());
            glVertex2d(max.x(), min.y());
            glVertex2d(max.x(), max.y());
            glVertex2d(min.x(), max.y());
          glEnd();
#endif
	  n_first = n_last + 1;
	  IP_prev = Inflection_Point(n, t, inflection_pt);
	}
      }
    }
    
    CP2_prev = CP2;
    
    // Last bounding box
    if (b == b_last) {
      CPs.push_back(CP3);
      vec2 min, max;
      getBoundingBox(IP_prev.pos, IP_prev.n, IP_prev.t,
		     (*b).V[3], n_tot, 1.0, min, max);
      n_last = CPs.size() - 1;
      correctCurvatureCenters(CPs, n_first, n_last, radiusMax(min, max),
			      first_box);
#if TEST_STROKE2D
      glColor3f(0.0, 0.0, 0.0); // Be careful!
      glBegin(GL_LINE_LOOP);
        glVertex2d(min.x(), min.y());
        glVertex2d(max.x(), min.y());
        glVertex2d(max.x(), max.y());
        glVertex2d(min.x(), max.y());
      glEnd();
#endif
    }
  }
  
  finalizeCurvatureCenters(bs, CPs);
}

/* TODO:
   . imposer le sens de courbure a l'aide des premiers points peut etre
    dangereux... Si erreur -> possibilite de tout retourner par correction ?
   . detecter si ligne droite -> grouper les bounding boxes ?
*/

const Stroke2D::real Stroke2D::steps_per_unit_length = 0.1; // Magic number!

Stroke2D::Stroke2D()
  : length(0.0) {
  const int n = 10; // Magic number!
  bs.reserve(n);
  relative_lengths.reserve(n);
}

Stroke2D::Stroke2D(Input& in, real error)
  : length(0.0) {
  if (in.positions.size() > 1) {
    const int n = 10; // Magic number!
    bs.reserve(n);
    relative_lengths.reserve(n);
    fit(in, error);
    evalLength();
#ifndef TEST_STROKE2D
    evalCurvatureCenters();
#endif
  }
}

void Stroke2D::read(ifstream& file_in) {
  char line[256];
  file_in.getline(line, 256, '\n');
  int n;
  sscanf(line, "%d", &n);
  bs.reserve(n);
  for (int i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    int m;
    sscanf(line, "%d", &m);
    bezier bez(m-1);
    for (int j = 0; j < m; j++) {
      file_in.getline(line, 256, '\n');
      real x, y;
      sscanf(line, "%lf %lf", &x, &y);
      bez.V[j] = vec2(x, y);
    }
    bs.push_back(bez);
  }
  relative_lengths.reserve(n);
  evalLength();
#ifndef TEST_STROKE2D
  evalCurvatureCenters();
#endif
}

void Stroke2D::write(ofstream& file_out) const {
  file_out << bs.size() << endl;
  beziers::const_iterator p;
  for (p = bs.begin(); p != bs.end(); p++) {
    file_out << (*p).V.size() << endl;
    bezier::ctrl_points::const_iterator cp;
    for (cp = (*p).V.begin(); cp != (*p).V.end(); cp++) {
      file_out << (*cp) << endl;
    }
  }
}

bool Stroke2D::empty() const {
  return bs.empty();
}

void Stroke2D::drawSpline() const {
  const int nstep = static_cast<int>(steps_per_unit_length*length);
  glBegin(GL_LINE_STRIP);
  vec2 Q;
  beziers::const_iterator p = bs.begin();
  beziers::const_iterator p_last = bs.end() - 1;
  int i = 0;
  for (; p != bs.end(); p++, i++) {
    int nstep_b = static_cast<int>(relative_lengths[i]*nstep);
    if (nstep_b == 0) {
      nstep_b = 1;
    }
    const real step_size = 1.0/nstep_b;
    int t_stop;
    if (p == p_last) {
      t_stop = nstep_b + 1;
    }
    else {
      t_stop = nstep_b;
    }
    for (int t = 0; t < t_stop; t++) {
      (*p).evaluate(t*step_size, Q);
      glVertex2d(Q.x(), Q.y());
    }
  }
  glEnd();
}

void Stroke2D::drawControlPoints() const {
  glBegin(GL_POINTS);
  beziers::const_iterator p;
  bezier::ctrl_points::const_iterator cp;
  for (p = bs.begin(); p != bs.end(); p++) {
    for (cp = (*p).V.begin(); cp != (*p).V.end(); cp++) {
      glVertex2d((*cp).x(), (*cp).y());
    }
  }
  glEnd();
}

void Stroke2D::drawTangents() const {
  glBegin(GL_LINES);
  beziers::const_iterator p;
  bezier::ctrl_points::const_iterator cp;
  for (p = bs.begin(); p != bs.end(); p++) {
    cp = (*p).V.begin();
    glVertex2d((*cp).x(), (*cp).y());
    glVertex2d((*(cp+1)).x(), (*(cp+1)).y());
    cp = (*p).V.end()-1;
    glVertex2d((*cp).x(), (*cp).y());
    glVertex2d((*(cp-1)).x(), (*(cp-1)).y());
  }
  glEnd();
}

void Stroke2D::drawDerivatives() const {
  const int nstep = static_cast<int>(steps_per_unit_length*length);
  glBegin(GL_LINES);
  vec2 Q, Q_prime, Q_second;
  beziers::const_iterator p = bs.begin();
  beziers::const_iterator p_last = bs.end() - 1;
  int i = 0;
  for (; p != bs.end(); p++, i++) {
    int nstep_b = static_cast<int>(relative_lengths[i]*nstep);
    if (nstep_b == 0) {
      nstep_b = 1;
    }
    const real step_size = 1.0/nstep_b;
    int t_stop;
    if (p == p_last) {
      t_stop = nstep_b + 1;
    }
    else {
      t_stop = nstep_b;
    }
    for (int t = 0; t < t_stop; t++) {
      (*p).evaluate(t*step_size, Q);
      (*p).evalDerivative(1, t*step_size, Q_prime);
      (*p).evalDerivative(2, t*step_size, Q_second);
      glColor3f(1.0, 0.0, 0.0); // Be careful!
      glVertex2d(Q.x(), Q.y());
      glVertex2d(Q.x()+Q_prime.x(), Q.y()+Q_prime.y());
      glColor3f(0.0, 1.0, 0.0); // Be careful!
      glVertex2d(Q.x(), Q.y());
      glVertex2d(Q.x()+Q_second.x(), Q.y()+Q_second.y());
    }
  }
  glEnd();
}

void Stroke2D::drawCurvatureVectors() const {
  glBegin(GL_LINES);
  beziers::const_iterator p;
  for (p = bs.begin(); p != bs.end(); p++) {
    bezier::ctrl_points::const_iterator cp = (*p).V.begin();
    bezier::curv_centers::const_iterator cc = (*p).C.begin();
    for (; cp != (*p).V.end(); cp++, cc++) {
      glVertex2d((*cp).x(), (*cp).y());
      glVertex2d((*cc).x(), (*cc).y());
    }
  }
  glEnd();
}
