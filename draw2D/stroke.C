#include "stroke.h"

Stroke::Point::Point() {}

Stroke::Point::Point(const vec2& v)
  : pos(v), u(0.0) {}

Stroke::Bezier::Bezier() {}

Stroke::Bezier::Bezier(int degree)
  : V(degree+1), length(0.0) {}

Stroke::Bezier::Bezier(const Bezier& b)
  : V(b.V), length(b.length) {}

/*
 *  Main source of inspiration for Bezier fitting:
 *  An Algorithm for Automatically Fitting Digitized Curves
 *  by Philip J. Schneider
 *  from Graphics Gems, Academic Press, 1990.
 */

/*
 *  Bernstein polynomials
 */
Stroke::real Stroke::Bezier::B30(real u) const {
  real tmp = 1.0 - u;
  return (tmp * tmp * tmp);
}
Stroke::real Stroke::Bezier::B31(real u) const {
  real tmp = 1.0 - u;
  return (3.0 * u * tmp * tmp);
}
Stroke::real Stroke::Bezier::B32(real u) const {
  real tmp = 1.0 - u;
  return (3.0 * u * u * tmp);
}
Stroke::real Stroke::Bezier::B33(real u) const {
  return (u * u * u);
}

/*
 *  evaluate :
 *  Evaluate a Bezier curve at a particular parameter value
 */
void Stroke::Bezier::evaluate(real t, vec2& Q) const {
  ctrl_points V_tmp(V);    /* Local copy of control points */
  int degree = V.size()-1; /* Degree of the Bezier curve */
  
  /* Triangle computation: de Casteljau's algorithm */
  ctrl_points::iterator p;
  for (int i = 1; i <= degree; i++)
    for (p = V_tmp.begin(); p != V_tmp.end()-i; p++)
      (*p) = (1.0 - t)*(*p) + (t)*(*(p+1));
  
  Q = V_tmp.front(); /* Point on curve at parameter t */
}

void Stroke::Bezier::evalDerivative(int order, real t, vec2& Q) const {
  ctrl_points V_tmp(V);    /* Local copy of control points */
  int degree = V.size()-1; /* Degree of the Bezier curve */
  
  /* Nth order differentiation of the de Casteljau's algorithm */
  ctrl_points::iterator p;
  for (int j = 1; j <= order; j++)
    for (p = V_tmp.begin(); p != V_tmp.end()-j; p++)
      (*p) = (*(p+1)) - (*p);
  for (int i = order+1; i <= degree; i++)
    for (p = V_tmp.begin(); p != V_tmp.end()-i; p++)
      (*p) = (1.0 - t)*(*p) + (t)*(*(p+1));
  
  int n = 1;
  int m = 1;
  for(j = 1; j <= degree; j++)
    n *= j;
  for(i = 1; i <= degree - order; i++)
    m *= i;                            // TO BE OPTIMIZED!
  
  Q = V_tmp.front()*(n/m); /* Nth order derivative at parameter t */
}

bool Stroke::Bezier::evalCurvatureVector(real t, vec2& Q, vec2& C) const {
  vec2 Q_prime, Q_second;
  evaluate(t, Q);
  evalDerivative(1, t, Q_prime);
  evalDerivative(2, t, Q_second);
  real tmp = dot(Q_prime, Q_prime)/cross(Q_prime, Q_second);
  if (Numerics<real>::isfinite(tmp)) {
    C.setx(Q.x() - Q_prime.y()*tmp); // Center of curvature
    C.sety(Q.y() + Q_prime.x()*tmp);
    return true;
  }
  else {
    C.setx(Q.x() - Q_prime.y());
    C.sety(Q.y() + Q_prime.x());
    return false;
  }
}

// IMPORTANT WARNING! IMPORTANT WARNING! IMPORTANT WARNING!
// All these functions use iterators on points... It works because points array
// is never modified... Beware! Better use array indices rather than iterators!

/*
 *  fitCurve :
 *  Fit a Bezier curve to a set of digitized points 
 */
void Stroke::fitCurve(points::iterator first, points::iterator last,
		      real error) {
  vec2 tanv1 = leftTangent(first); /* Unit tangent vectors at endpoints */
  vec2 tanv2 = rightTangent(last);
  fitCubic(first, last, tanv1, tanv2, error);
}

/*
 *  fitCubic :
 *  Fit a Bezier curve to a (sub)set of digitized points
 */
void Stroke::fitCubic(points::iterator first, points::iterator last,
		      vec2 tanv1, vec2 tanv2, real error) {
  Bezier b(3); /* Fitted cubic Bezier curve */
  //beziers bs; // temp
  
  /* Number of points in subset */
  points::difference_type nPts = std::distance(first, last) + 1;
  
  /*  Use heuristic if region only has two points in it */
  if (nPts == 2) {
    real d = dist((*last).pos, (*first).pos)/3.0;
    b.V[0] = (*(first)).pos;
    b.V[3] = (*(last)).pos;
    b.V[1] = b.V[0] + tanv1*d;
    b.V[2] = b.V[3] + tanv2*d;
    bs.push_back(b);
    //hierarchy.push_back(bs);
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
    //hierarchy.push_back(bs);
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
	//hierarchy.push_back(bs);
	return;
      }
    }
  }
  
  /* Fitting failed -- split at max error point and fit recursively */
  vec2 tanv_center = centerTangent(split); /* Unit tangent vector */
                                           /* at split point */
  fitCubic(first, split, tanv1, tanv_center, error);
  fitCubic(split, last, -tanv_center, tanv2, error);
}

/*
 *  generateBezier :
 *  Use least-squares method to find Bezier control points for region.
 */
void Stroke::generateBezier(points::const_iterator first,
			    points::const_iterator last,
			    vec2 tanv1, vec2 tanv2, Bezier& b) {
  std::vector<A> As; /* Precomputed rhs for eqn */
  
  /* Compute the A's */
  points::const_iterator p;
  for (p = first; p <= last; p++) {
    real u = (*p).u;
    As.push_back(A(tanv1*b.B31(u),tanv2*b.B32(u)));
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
               - ((*first).pos*b.B30(u) + (*first).pos*b.B31(u) +
		   (*last).pos*b.B32(u) + (*last).pos*b.B33(u));
    
    X[0] += dot((*a).A1,tmp);
    X[1] += dot((*a).A2,tmp);
  }
  
  /* Compute the determinants of C and X */
  real det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
  det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];
  
  /* Finally, derive alpha values */
  if (det_C0_C1 == 0.0) {
    det_C0_C1 = (C[0][0] * C[1][1]) * 1.0e-12;
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
void Stroke::findNewtonRaphsonRoot(Bezier& Q, Point& P) {
  vec2 Q_u, Q1_u, Q2_u; /* u evaluated at Q, Q' & Q'' */
  
  /* Compute Q(u) */
  Q.evaluate(P.u, Q_u);
  
  /* Generate control vertices for Q' */
  Bezier Q1(2);
  Bezier::ctrl_points::iterator p1 = Q1.V.begin();
  Bezier::ctrl_points::iterator p  = Q.V.begin();
  for (; p1 != Q1.V.end(); p1++, p++) {
    (*p1) = ((*(p+1)) - (*p)) * 3.0;
  }
  
  /* Generate control vertices for Q'' */
  Bezier Q2(1);
  Bezier::ctrl_points::iterator p2 = Q2.V.begin();
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
void Stroke::computeMaxError(points::iterator first, points::iterator last,
			     Bezier& b, points::iterator& split,
			     real* maxError) {
  points::difference_type splitPoint = 0.5*(std::distance(first, last) + 1);
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
 */
Stroke::vec2 Stroke::leftTangent(points::const_iterator p) {
  //return ((*(p+1)).pos - (*p).pos).normalize();
  return (0.5*((*(p+2)).pos + (*(p+1)).pos - 2.0*(*p).pos)).normalize();
}
Stroke::vec2 Stroke::rightTangent(points::const_iterator p) {
  //return ((*(p-1)).pos - (*p).pos).normalize();
  return (0.5*((*(p-2)).pos + (*(p-1)).pos - 2.0*(*p).pos)).normalize();
}
Stroke::vec2 Stroke::centerTangent(points::const_iterator p) {
  return ((*(p-1)).pos - (*(p+1)).pos).normalize();
}

/*
 *  chordLengthParameterize :
 *  Assign parameter values to digitized points 
 *  using relative distances between points.
 */
void Stroke::chordLengthParameterize(points::iterator first,
				     points::iterator last) {
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
void Stroke::reparameterize(points::iterator first, points::iterator last,
			    Bezier& b) {
  for (points::iterator p = first; p <= last; p++)
    findNewtonRaphsonRoot(b, (*p));
}

Stroke::Stroke()
  : length(0.0), pixel_size(0.0) {
  bs.reserve(10);
}

Stroke::Stroke(Input& in, real error) : length(0.0) {
  bs.reserve(10);
  pixel_size = in.pixelSize();
  if (in.positions().size() > 1) {
    // Copying data
    points pts;
    std::vector<vec2>::const_iterator v;
    if (in.positions().size() > 2) {
      pts.push_back(Point(in.positions().front()));
      // Filtering with a little convolution
      for (v = in.positions().begin()+1; v != in.positions().end()-1; v++)
	pts.push_back(Point(0.0625*(*(v-1)) + 0.875*(*v) + 0.0625*(*(v+1))));
      pts.push_back(Point(in.positions().back()));
    }
    else
      for (v = in.positions().begin(); v != in.positions().end(); v++)
	pts.push_back(Point(*v));
    
    // Locating corners
    points::iterator first = pts.begin();
    points::iterator last  = pts.end()-1;
    points::iterator p;
    points::iterator split = pts.begin()+1;
    bool have_corner = false;
    for (p = first+1; p != last; p++) {
      real cos_a =
	cosAng((*(p+1)).pos - (*p).pos, (*(p-1)).pos - (*p).pos);
      if (cos_a > 0.0) {
	have_corner = true;
        split = p;
        fitCurve(first, split, error);
        first = split;
      }
    }
    if (have_corner)
      fitCurve(split, last, error);
    else
      fitCurve(pts.begin(), pts.end()-1, error);
    
    // Length
    beziers::iterator b;
    for (b = bs.begin(); b != bs.end(); b++) {
      vec2 Q_prev, Q_curr;
      (*b).evaluate(0, Q_prev);
      for (int t = 1; t <= 10; t++) { // Magic number!
	(*b).evaluate(t*0.1, Q_curr);
	(*b).length += dist(Q_prev, Q_curr);
	Q_prev = Q_curr;
      }
      length += (*b).length;
    }
  }
}

bool Stroke::empty() const {
  return bs.empty();
}

void Stroke::drawSpline() const {
  glBegin(GL_LINE_STRIP);
    vec2 Q;
    beziers::const_iterator p;
    for (p = bs.begin(); p != bs.end(); p++) {
      for (int t = 0; t <= 10; t++) { // Magic number!
	(*p).evaluate(t*0.1, Q);
	glVertex2d(Q.x(), Q.y());
      }
    }
  glEnd();
}

void Stroke::drawCtrlPts() const {
  glBegin(GL_POINTS);
    beziers::const_iterator p;
    Bezier::ctrl_points::const_iterator cp;
    for (p = bs.begin(); p != bs.end(); p++)
      for (cp = (*p).V.begin(); cp != (*p).V.end(); cp++)
	glVertex2d((*cp).x(), (*cp).y());
  glEnd();
}

void Stroke::drawTgts() const {
  glBegin(GL_LINES);
    beziers::const_iterator p;
    Bezier::ctrl_points::const_iterator cp;
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

void Stroke::drawDerivatives() const {
  glBegin(GL_LINES);
    vec2 Q, Q_prime, Q_second;
    beziers::const_iterator p;
    for (p = bs.begin(); p != bs.end(); p++)
      for (int t = 0; t <= 10; t++) { // Magic number!
	(*p).evaluate(t*0.1, Q);
	(*p).evalDerivative(1, t*0.1, Q_prime);
	(*p).evalDerivative(2, t*0.1, Q_second);
	glColor3f(1.0,0.0,0.0);
	glVertex2d(Q.x(), Q.y());
	glVertex2d(Q.x()+Q_prime.x(), Q.y()+Q_prime.y());
	glColor3f(0.0,1.0,0.0);
	glVertex2d(Q.x(), Q.y());
	glVertex2d(Q.x()+Q_second.x(), Q.y()+Q_second.y());
      }
  glEnd();
}

void minmax(const Vec2<double>& value, Vec2<double>& min, Vec2<double>& max) {
  double value_x = value.x();
  double value_y = value.y();
  if (value_x < min.x())
    min.setx(value_x);
  else if (value_x > max.x())
    max.setx(value_x);
  if (value_y < min.y())
    min.sety(value_y);
  else if (value_y > max.y())
    max.sety(value_y);
}

void Stroke::drawCurvatureVector() const {
  bool is_not_first = false;
  bool first_undone = true;
  bool last_done = false;
  bool to_be_reversed = false;
  vec2 Q, C, last_inflection_pt, front_vel, curr_vel;
  vec2 min, max;
  std::vector<Curvature_Vector> CVs;
  CVs.reserve(100);
  int CVs_index = 0; // CVs array index
  beziers::const_iterator bb = bs.begin();
  const beziers::const_iterator b_first = bs.begin();
  const beziers::const_iterator b_last = bs.end()-1;
  // nstep_tot and nstep always > 0
  int nstep_tot = 0.1*(length/pixel_size) + 1; // Magic number!
  for (beziers::const_iterator b = bs.begin(); b != bs.end(); b++) {
    int nstep = ((*b).length/length)*nstep_tot + 1;
    real step_size = 1.0/nstep;
    if (b == b_last)
      nstep = nstep + 1;
    int t_stop = nstep - 1;
    for (int t = 0; t < nstep; t++) {
      bool is_number = (*b).evalCurvatureVector(t*step_size, Q, C);
      Curvature_Vector CV(Q, C, is_number);
      if (is_not_first) {
	// Detect if last point
	if (b == b_last && t == t_stop) {
	  // Find bounding box
	  min = Q;
	  max = Q;
	  minmax(last_inflection_pt, min, max);
	  if (last_done) {
	    minmax((*bb).V[2], min, max);
	    minmax((*bb).V[3], min, max);
	  }
	  else {
	    if (b_first == b_last) {
	      minmax((*bb).V[0], min, max);
	      minmax((*bb).V[1], min, max);
	      minmax((*bb).V[2], min, max);
	      minmax((*bb).V[3], min, max);
	    }
	    else {
	      bool first = true;
	      for (; bb != b; bb++) {
		if (first) {
		  minmax((*bb).V[2], min, max);
	          minmax((*bb).V[3], min, max);
		}
		else {
		  minmax((*bb).V[0], min, max);
	          minmax((*bb).V[1], min, max);
	          minmax((*bb).V[2], min, max);
	          minmax((*bb).V[3], min, max);
		}
	      }
	      minmax((*bb).V[0], min, max);
	      minmax((*bb).V[1], min, max);
	      minmax((*bb).V[2], min, max);
	      minmax((*bb).V[3], min, max);
	    }
	  }
#if DEBUG
	  glBegin(GL_LINE_LOOP);
	    glVertex2d(min.x(), min.y());
	    glVertex2d(max.x(), min.y());
	    glVertex2d(max.x(), max.y());
	    glVertex2d(min.x(), max.y());
	  glEnd();
#endif
          // Correct curvature vector norm
	  CVs.push_back(CV);
          real radius_max;
          real rx = max.x() - min.x();
          real ry = max.y() - min.y();
          if (rx > ry)
            radius_max = 0.5*rx; // Magic number!
          else
            radius_max = 0.5*ry;
          for (int i = CVs_index; i < CVs.size(); i++) {
            if (!CVs[i].is_finite || CVs[i].R > radius_max) {
              if (to_be_reversed)
                CVs[i].v = -(CVs[i].v/CVs[i].R)*radius_max;
              else
                CVs[i].v = (CVs[i].v/CVs[i].R)*radius_max;
              CVs[i].C = CVs[i].Q + CVs[i].v;
            }
            else
              if (to_be_reversed) {
                CVs[i].v = -CVs[i].v;
                CVs[i].C = CVs[i].Q + CVs[i].v;
              }
          }
	  (*b_last).evalDerivative(1, 1.0, curr_vel);
	  // Normally dot is used here!
          if (cross(front_vel, CVs.front().v)*
	      cross(curr_vel, CVs.back().v) < 0.0) {
            CVs.back().v = -CVs.back().v;
            CVs.back().C = CVs.back().Q + CVs.back().v;
          }
        }
        // Detect inflection point
	else if (dot(CV.v, CVs.back().v) < 0.0) {
	  // Find bounding box
	  min = Q;
	  max = Q;
	  minmax(last_inflection_pt, min, max);
	  if (first_undone) {
	    if (b == b_first) {
	      first_undone = false;
	      minmax((*bb).V[0], min, max);
	      minmax((*bb).V[1], min, max);
	    }
	    else {
	      first_undone = false;
	      for (; bb != b; bb++) {
		minmax((*bb).V[0], min, max);
	        minmax((*bb).V[1], min, max);
	        minmax((*bb).V[2], min, max);
	        minmax((*bb).V[3], min, max);
	      }
	      minmax((*bb).V[0], min, max);
	      minmax((*bb).V[1], min, max);
	    }
	  }
	  else {
	    bool first = true;
	    for (; bb != b; bb++) {
	      if (first) {
		first = false;
		minmax((*bb).V[2], min, max);
	        minmax((*bb).V[3], min, max);
	      }
	      else {
	        minmax((*bb).V[0], min, max);
	        minmax((*bb).V[1], min, max);
	        minmax((*bb).V[2], min, max);
	        minmax((*bb).V[3], min, max);
	      }
	    }
	    minmax((*bb).V[0], min, max);
	    minmax((*bb).V[1], min, max);
	  }
	  last_inflection_pt = Q;
	  if (b == b_last)
	    last_done = true;
#if DEBUG
	  glBegin(GL_LINE_LOOP);
	    glVertex2d(min.x(), min.y());
	    glVertex2d(max.x(), min.y());
	    glVertex2d(max.x(), max.y());
	    glVertex2d(min.x(), max.y());
	  glEnd();
#endif
	  // Correct curvature vector norm
          real radius_max;
          real rx = max.x() - min.x();
          real ry = max.y() - min.y();
          if (rx > ry)
            radius_max = 0.5*rx; // Magic number!
          else
            radius_max = 0.5*ry;
          for (int i = CVs_index; i < CVs.size(); i++) {
            if (!CVs[i].is_finite || CVs[i].R > radius_max) {
              if (to_be_reversed)
                CVs[i].v = -(CVs[i].v/CVs[i].R)*radius_max;
              else
                CVs[i].v = (CVs[i].v/CVs[i].R)*radius_max;
              CVs[i].C = CVs[i].Q + CVs[i].v;
            }
            else
              if (to_be_reversed) {
                CVs[i].v = -CVs[i].v;
                CVs[i].C = CVs[i].Q + CVs[i].v;
              }
          }
          CVs.push_back(CV);
          CVs_index = CVs.size() - 1;
	  (*b).evalDerivative(1, t*step_size, curr_vel);
	  // Normally dot is used here!
          if (cross(front_vel, CVs.front().v)*cross(curr_vel, CV.v) < 0.0)
            to_be_reversed = true;
          else
            to_be_reversed = false;
        }
        else
          CVs.push_back(CV);
      }
      else {
	is_not_first = true;
	last_inflection_pt = Q;
	(*b_first).evalDerivative(1, 0.0, front_vel);
	CVs.push_back(CV);
      }
    }
  }
  glBegin(GL_LINES);
    glColor3f(0.0,0.0,1.0);
    std::vector<Curvature_Vector>::iterator c;
    for (c = CVs.begin(); c != CVs.end(); c++) {
      glVertex2d((*c).Q.x(), (*c).Q.y());
      glVertex2d((*c).C.x(), (*c).C.y());
    }
  glEnd();
}
/* TODO:
   .detecter si ligne droite -> grouper les bounding boxes ?
   .imposer le sens de courbure a l'aide des premiers points peut etre
   dangereux... Si erreur -> possibilite de tout retourner par correction ?
   .ou bien faire un vote pour savoir combien de traits sont a l'endroit/envers
   (nb de voix plus important pour le debut de la courbe ?)
*/
