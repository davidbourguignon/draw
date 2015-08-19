#include "stroke3D.h"

using namespace std;

void Stroke3D::computeBezierSurface() {
  /* Compute two cubic beziers */
  beziers_simples b_plus_psang;
  beziers_simples b_minus_psang;
  b_plus_psang.reserve(bs.size());
  b_minus_psang.reserve(bs.size());
  for (beziers::const_iterator p = bs.begin(); p != bs.end(); p++) {
    const int degree = (*p).C.size() - 1; // Optimization needed?
    bezier_simple bez_plus_psang(degree);
    bezier_simple bez_minus_psang(degree);
    for (int i = 0; i < degree + 1; i++) {
      bezier::vec  C = (*p).C[i];
      bezier::real R = (*p).R[i];
      bezier::vec  N = (*p).N[i];
      bez_plus_psang.V[i]  = C + R*(cos_psang*N + sin_psang*plane_normal);
      bez_minus_psang.V[i] = C + R*(cos_psang*N - sin_psang*plane_normal);
    }
    b_plus_psang.push_back(bez_plus_psang);
    b_minus_psang.push_back(bez_minus_psang);
  }
  
  /* Compute bezier surface */
  beziers::const_iterator p = bs.begin();
  beziers_simples::const_iterator p_plus  = b_plus_psang.begin();
  beziers_simples::const_iterator p_minus = b_minus_psang.begin();
  for (; p != bs.end(); p++, p_plus++, p_minus++) {
    bezier_surface bez_surf;
    bez_surf.V = (*p_plus).V;
    
    // Compute circle arcs as quadratic bezier curves
    beziers_simples circles;
    circles.reserve((*p).V.size());
    for (int i = 0; i < (*p).C.size(); i++) {
      bezier_simple circle(2);
      bezier::vec  C = (*p).C[i];
      bezier::real R = (*p).R[i];
      bezier::vec  N = (*p).N[i];
      circle.V[0] = C + R*(cos_psang*N + sin_psang*plane_normal);
      circle.V[2] = C + R*(cos_psang*N - sin_psang*plane_normal);
      /*
	 The isosceles triangle property is only valid for rational Beziers,
         so we only get an approximation here
      */
      vec3 top = circle.V[0] + ((circle.V[2] - circle.V[0]).norm())*
                                (sin_psang*(*p).N[i] - cos_psang*plane_normal);
      vec3 half_base = 0.5*(circle.V[2] + circle.V[0]);
      vec3 height = top - half_base;
      /*
         Best approximation! But depend of psang! Here we take psang = PI/3.
      */
      const real best_approx = 0.698393;
      circle.V[1] = half_base + best_approx*height;
      circles.push_back(circle);
    }
    beziers_simples::const_iterator pc = circles.begin();
    for (; pc != circles.end(); pc++) {
      bez_surf.V.push_back((*pc).V[1]);
    }
    bez_surf.V.insert(bez_surf.V.end(), (*p_minus).V.begin(),
		                        (*p_minus).V.end());
    bez_surf.order_u = 4;
    bez_surf.order_v = 3;
    proba_surface.push_back(bez_surf);
  }
}

void Stroke3D::computeBoundingBox() {
  const int nstep_v = 3;
  const real stepsize_v = 1.0/nstep_v;
  vec3 Q;
  std::vector<vec3> points;
  int index = 0;
  for (beziers_surfaces::const_iterator ps = proba_surface.begin();
       ps != proba_surface.end(); ps++, index++) {
    int nstep_u = static_cast<int>(0.5*nsteps[index]); // Magic number!
    if (nstep_u == 0) {
      nstep_u = 1;
    }
    const real stepsize_u = 1.0/nstep_u;
    for (int i = 0; i < nstep_u + 1; i++) {
      for (int j = 0; j < nstep_v + 1; j++) {
	(*ps).evaluate(i*stepsize_u, j*stepsize_v, Q);
	points.push_back(Q);
      }
    }
  }
  box = bounding_box(points);
}

void Stroke3D::computeBarycenter() {
  int count = 0;
  barycenter_global = bezier::vec::null();
  beziers::const_iterator p_last = bs.end() - 1;
  for (beziers::const_iterator p = bs.begin(); p != bs.end(); p++) {
    int c_stop;
    if (p == p_last) {
      c_stop = (*p).C.size();
    }
    else {
      c_stop = (*p).C.size() - 1;
    }
    for (int c = 0; c < c_stop; c++) {
      barycenter_global += (*p).C[c];
      count++;
    }
  }
  barycenter_global /= count;
}

void Stroke3D::probaSurface(/*const GLint nstep_u, */const GLint nstep_v,
			    const int texture_mode) const {
  glPushAttrib(GL_EVAL_BIT);
  
  GLdouble param_prev = 0.0;
  int index = 0;
  for (beziers_surfaces::const_iterator ps = proba_surface.begin();
       ps != proba_surface.end(); ps++, index++) {
    // Tex coords
    if (texture_mode == NO_TEXTURE) {}
    else if (texture_mode == TEXTURE_1D) {
      static const GLdouble tex_points[2][2][1] = {{{0.0}, {0.0}},
                                                   {{1.0}, {1.0}}};
      glMap2d(GL_MAP2_TEXTURE_COORD_1, 0.0, 1.0, 1, 2, 0.0, 1.0, 2, 2,
              &tex_points[0][0][0]);
    }
    else if (texture_mode == TEXTURE_2D) {
      const GLdouble rel_len = relative_lengths[index];
      const GLdouble param   = param_prev + rel_len;
      const GLdouble tex_points[2][2][2] = {{{0.0, param_prev}, {0.0, param}},
					    {{1.0, param_prev}, {1.0, param}}};
      param_prev += rel_len;
      glMap2d(GL_MAP2_TEXTURE_COORD_2, 0.0, 1.0, 2, 2, 0.0, 1.0, 4, 2,
              &tex_points[0][0][0]);
    }
    else {
      assert(false);
    }
    
    const GLint nstep_u = nsteps[index];
    (*ps).draw(nstep_u, nstep_v);
  }
  
  glPopAttrib();
}
/* TODO:
   Use float instead of double to minimize memory cost.
*/

void Stroke3D::initSteps() {
  const GLint nstep_tot = static_cast<GLint>(steps_per_unit_length*length);
  beziers::const_iterator p = bs.begin();
  beziers::const_iterator p_last = bs.end() - 1;
  int i = 0;
  for (; p != bs.end(); p++, i++) {
    int nstep_b = static_cast<int>(relative_lengths[i]*nstep_tot);
    if (nstep_b < 1) {
      nstep_b = 1;
    }
    nsteps.push_back(nstep_b);
  }
}

void Stroke3D::computeMeanRadius() {
  mean_radius = 0.0;
  beziers::const_iterator p_last = bs.end() - 1;
  int count = 0;
  for (beziers::const_iterator p = bs.begin(); p != bs.end(); p++) {
    int r_stop;
    if (p == p_last) {
      r_stop = (*p).R.size();
    }
    else {
      r_stop = (*p).R.size() - 1;
    }
    for (int r = 0; r < r_stop; r++) {
      mean_radius += (*p).R[r];
      count++;
    }
  }
  mean_radius /= count;
}

void Stroke3D::computeNormals() {
#if 0
  mean_normal = vec3::null();
  int count = 0;
#endif
  for (beziers::iterator p = bs.begin(); p != bs.end(); p++) {
    (*p).computeNormals();
#if 0
    bezier::normals::iterator np = (*p).N.begin();
    for (; np != (*p).N.end(); np++) {
      mean_normal += (*np);
      count++;
    }
#endif
  }
#if 0
  mean_normal /= count;
  mean_normal.normalize();
  ortho_plane_normal = cross(mean_normal, plane_normal);
#endif
}
/* TO DO:
   . Suppress mean normal, ortho plane normal -> useless!
*/

void Stroke3D::buildDisplayLists(const int window) {
  glutSetWindow(window);
  //const GLint nu = 4; // Magic number!
  const GLint nv = 4; // Magic number!
  proba_surface_list = glGenLists(1);
  if (proba_surface_list) {
    glNewList(proba_surface_list, GL_COMPILE);
    probaSurface(/*nu, */nv, TEXTURE_2D);
    glEndList();
  }
  else {
    assert(false);
  }
  proba_surface_picking_list = glGenLists(1);
  if (proba_surface_picking_list) {
    glNewList(proba_surface_picking_list, GL_COMPILE);
    //glPushAttrib(GL_ENABLE_BIT);
    //glEnable(GL_CULL_FACE);
    probaSurface(/*nu, */nv, NO_TEXTURE);
    //glPopAttrib();
    glEndList();
  }
  else {
    assert(false);
  }
}
/* TODO:
   I don't see an obvious solution for determining if front face must be
   reversed. It is the necessary condition for back face culling to work
   properly.
*/

void Stroke3D::initDisplayData() {
  /* Clipping planes equations */
  std::vector<GLdouble> eqn(4, 0.0);
  equations.push_back(eqn);
  equations.push_back(eqn);
  setClippingPlanesEqns();
}

void Stroke3D::setClippingPlanesEqns() {
  Vec3<GLdouble> normal_curr = - view_vector_prev; // current in fact!
  // Orthographic camera hypothesis!
  std::vector< Vec3<GLdouble> > points;
  std::vector< Vec3<GLdouble> > normals;
  const GLdouble ratio = /*0.25*/0.2; // Magic number!
  Vec3<GLdouble> point_offset = ratio*mean_radius*normal_curr;
  points.push_back(barycenter_global + point_offset);
  points.push_back(barycenter_global - point_offset);
  normals.push_back(-normal_curr);
  normals.push_back( normal_curr);
  for (int i = 0; i < equations.size(); i++) {
    equations[i][3] = 0.0;
    for (int j = 0; j < 3; j++) {
      equations[i][j]  = normals[i][j];
      equations[i][3] -= normals[i][j]*points[i][j];
    }
  }
}

/*****************************************************************************/

const Stroke3D::real Stroke3D::psang     = M_PI/3.0;
const Stroke3D::real Stroke3D::cos_psang = 0.5;
const Stroke3D::real Stroke3D::sin_psang = 0.5*Numerics<real>::sqroot(3.0);
const Stroke3D::real Stroke3D::steps_per_unit_length = 0.05; // Magic numbers!

Stroke3D::Stroke3D()
  : view_vector_prev(vec3::null()), length(0.0),
    plane_normal(vec3::null()), mean_radius(0.0),
    occluder_tex_name(0), proba_surface_tex_name(0), stroke_tex_name(0),
    drawing_mode(0) {}

Stroke3D::Stroke3D(const Input& in, const Stroke2D& s, const int mode)
  : occluder_tex_name(0), proba_surface_tex_name(0), stroke_tex_name(0),
    drawing_mode(mode) {
  
  if (!s.empty()) {
    const int size = s.bs.size();
    bs.reserve(size);
    
    /* Projection in 3D */
    const GLdouble winz_first = in.getFirstPlane();
    const GLdouble winz_last = in.getLastPlane();
    const int mode = in.projectionMode();
    
    if ((mode == Input::FOLLOW) || (mode == Input::SPLAT)) {
      GLdouble winz = winz_first;
      Stroke2D::beziers::const_iterator b = s.bs.begin();
      for (; b != s.bs.end(); b++) {
        bezier bez;
        Stroke2D::bezier::ctrl_points::const_iterator cp = (*b).V.begin();
	for (; cp != (*b).V.end(); cp++) {
	  GLdouble objx, objy, objz;
          if (gluUnProject((*cp).x(),
			   static_cast<GLdouble>(in.viewport[3]) - (*cp).y()
			   - 1.0,
		           winz,
		           in.mv_matrix, in.proj_matrix, in.viewport,
		           &objx, &objy, &objz)) {
	    bez.V.push_back(vec3(objx, objy, objz));
	  }
	  else {
	    assert(false);
	  }
        }
        bez.length = (*b).length;
        bez.T = (*b).T;
	Stroke2D::bezier::curv_centers::const_iterator cc = (*b).C.begin();
        for (; cc != (*b).C.end(); cc++) {
	  GLdouble objx, objy, objz;
          if (gluUnProject((*cc).x(),
			   static_cast<GLdouble>(in.viewport[3]) - (*cc).y()
			   - 1.0,
		           winz,
		           in.mv_matrix, in.proj_matrix, in.viewport,
		           &objx, &objy, &objz)) {
	    bez.C.push_back(vec3(objx, objy, objz));
	  }
	  else {
	    assert(false);
	  }
        }
        bez.computeRadii();
        bs.push_back(bez);
      }
      
      /* Plane normal computation */
      vec3 view_vector;
      writeViewVector(in.mv_matrix, view_vector);
      view_vector_prev = view_vector;
      plane_normal = - view_vector;
    }
    else if (mode == Input::BRIDGE) {
      Stroke2D::beziers::const_iterator b = s.bs.begin();
      const Stroke2D::beziers::const_iterator b_last = s.bs.end() - 1;
      int index = 0;
      real length_curr = 0.0;
      for (; b != s.bs.end(); b++, index++) {
	bezier bez;
        Stroke2D::bezier::ctrl_points::const_iterator cp = (*b).V.begin();
	Stroke2D::bezier::ctrl_points::const_iterator cp_end = (*b).V.end();
	Stroke2D::bezier::parameters::const_iterator ct = (*b).T.begin();
	Stroke2D::bezier::curv_centers::const_iterator cc = (*b).C.begin();
        for (; cp != cp_end; cp++, ct++, cc++) {
	  const GLdouble t = length_curr + (*ct)*s.relative_lengths[index];
	  GLdouble winz = (1.0 - t)*winz_first + (t)*winz_last;
	  GLdouble objx, objy, objz;
          if (gluUnProject((*cp).x(),
			   static_cast<GLdouble>(in.viewport[3]) - (*cp).y()
			   - 1.0,
		           winz,
		           in.mv_matrix, in.proj_matrix, in.viewport,
		           &objx, &objy, &objz)) {
	    bez.V.push_back(vec3(objx, objy, objz));
	  }
	  else {
	    assert(false);
	  }
	  if (gluUnProject((*cc).x(),
			   static_cast<GLdouble>(in.viewport[3]) - (*cc).y()
			   - 1.0,
		           winz,
		           in.mv_matrix, in.proj_matrix, in.viewport,
		           &objx, &objy, &objz)) {
	    bez.C.push_back(vec3(objx, objy, objz));
	  }
	  else {
	    assert(false);
	  }
        }
	length_curr += s.relative_lengths[index];
        bez.length = (*b).length;
        bez.T = (*b).T;
	bez.computeRadii();
        bs.push_back(bez);
      }
      
      /* Plane normal computation */
      int winx_first = static_cast<int>(in.positions.front().pos.x());
      int winy_first = static_cast<int>(in.positions.front().pos.y());
      int winx_last  = static_cast<int>(in.positions.back().pos.x());
      int winy_last  = static_cast<int>(in.positions.back().pos.y());
      vec3 first, last;
      GLdouble objx, objy, objz;
      if (gluUnProject(static_cast<GLdouble>(winx_first),
		       static_cast<GLdouble>(in.viewport[3] - winy_first - 1),
		       winz_first,
		       in.mv_matrix, in.proj_matrix, in.viewport,
		       &objx, &objy, &objz)) {
	first = vec3(objx, objy, objz);
      }
      else {
	assert(false);
      }
      if (gluUnProject(static_cast<GLdouble>(winx_last),
		       static_cast<GLdouble>(in.viewport[3] - winy_last - 1),
		       winz_last,
		       in.mv_matrix, in.proj_matrix, in.viewport,
		       &objx, &objy, &objz)) {
	last = vec3(objx, objy, objz);
      }
      else {
	assert(false);
      }
      vec3 bridge_vector = last - first;
      vec3 view_vector;
      writeViewVector(in.mv_matrix, view_vector);
      view_vector_prev = view_vector;
      real d = dot(view_vector, bridge_vector);
      if (d < 0.0) {
	bridge_vector = -bridge_vector;
	d = dot(view_vector, bridge_vector);
      }
      vec3 view_vector_new = view_vector - d*bridge_vector.normalize();
      plane_normal = - view_vector_new;
    }
    
    length = s.length;
    relative_lengths = s.relative_lengths;
    initSteps();
    computeMeanRadius();
#if 0
    if ((mode == Input::SPLAT) && (drawing_mode != LINE)) {
      const real factor = 2.0/3.0; // Magic number!
      vec3 transl_vector = factor*mean_radius*plane_normal;
      for (beziers::iterator p = bs.begin(); p != bs.end(); p++) {
	bezier::ctrl_points::iterator v = (*p).V.begin();
	bezier::curv_centers::iterator c = (*p).C.begin();
	for (; v != (*p).V.end(); v++, c++) {
	  (*v) += transl_vector;
	  (*c) += transl_vector;
	}
      }
    }
#endif
    computeNormals();
    computeBezierSurface();
    computeBoundingBox();
    computeBarycenter();
    buildDisplayLists(in.window);
    initDisplayData();
  }
}

void Stroke3D::setInitColor(const GLfloat c[4]) {
  color_init[0] = c[0]; color_init[1] = c[1];
  color_init[2] = c[2]; color_init[3] = c[3];
  reinitColor();
}

void Stroke3D::setColor(const GLfloat c[4]) {
  color[0] = c[0]; color[1] = c[1]; color[2] = c[2]; color[3] = c[3];
}

void Stroke3D::reinitColor() {
  color[0] = color_init[0]; color[1] = color_init[1];
  color[2] = color_init[2]; color[3] = color_init[3];
}

void Stroke3D::read(ifstream& file_in, const int window) {
  char line[256];
  file_in.getline(line, 256, '\n');
  int n;
  sscanf(line, "%d", &n);
  bs.reserve(n);
  relative_lengths.reserve(n);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf", &length);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf %lf %lf",
	 &plane_normal[0], &plane_normal[1], &plane_normal[2]);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf", &mean_radius);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%d", &drawing_mode);
  file_in.getline(line, 256, '\n');
  GLfloat c[4];
  sscanf(line, "%f %f %f %f", &c[0], &c[1], &c[2], &c[3]);
  setInitColor(c);
  
  for (int i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    real rl;
    sscanf(line, "%lf", &rl);
    relative_lengths.push_back(rl);
    bezier bez;
    bez.read(file_in);
    bs.push_back(bez);
  }
  
  initSteps();
  computeMeanRadius();
  computeNormals();
  computeBezierSurface();
  computeBoundingBox();
  computeBarycenter();
  buildDisplayLists(window);
  initDisplayData();
}

void Stroke3D::write(ofstream& file_out) const {
  file_out << bs.size() << endl;
  file_out << length << endl;
  file_out << plane_normal << endl;
  file_out << mean_radius << endl;
  file_out << drawing_mode << endl;
  file_out << color_init[0] << " " << color_init[1] << " "
	   << color_init[2] << " " << color_init[3] << endl;
  
  beziers::const_iterator b_p = bs.begin();
  std::vector<real>::const_iterator rl_p = relative_lengths.begin();
  for (; b_p != bs.end(); b_p++, rl_p++) {
    file_out << (*rl_p) << endl;
    (*b_p).write(file_out);
  }
}

bool Stroke3D::empty() const {
  return bs.empty();
}

void Stroke3D::move(const Input& in) {
  const vec3 view_vector(in.view_vector);
  if (view_vector == view_vector_prev) {
    return;
  }
  else {
    view_vector_prev = view_vector;
    
    /* Clipping planes */
    setClippingPlanesEqns();
  }
}

void Stroke3D::reverse(const int window) {
  for (beziers::iterator p = bs.begin(); p != bs.end(); p++) {
    bezier::curv_centers::iterator pc = (*p).C.begin();
    bezier::curv_centers::iterator pc_end = (*p).C.end();
    bezier::radii::const_iterator pr = (*p).R.begin();
    bezier::normals::iterator pn = (*p).N.begin();
    for (; pc != pc_end; pc++, pr++, pn++) {
      (*pc) += 2.0*(*pr)*(*pn); // Translate center of 2*radius along normal
      (*pn) = -(*pn);           // Reverse normal
    }
  }
  
  computeNormals();
  proba_surface.clear();
  computeBezierSurface();
  computeBoundingBox();
  computeBarycenter();
  clean(window);
  buildDisplayLists(window);
  initDisplayData();
}

void Stroke3D::translate(int first_x, int first_y, int last_x, int last_y,
			 const Input& in) {
  GLdouble winx, winy, winz;
  if (gluProject(barycenter_global[0],
		 barycenter_global[1],
		 barycenter_global[2],
		 in.mv_matrix, in.proj_matrix, in.viewport,
		 &winx, &winy, &winz)) {
    assert(winz >= 0.0 && winz <= 1.0);
  }
  else {
    assert(false);
  }
  GLdouble objx, objy, objz;
  vec3 first, last;
  if (gluUnProject(static_cast<GLdouble>(first_x),
		   static_cast<GLdouble>(in.viewport[3] - 1 - first_y),
		   winz,
		   in.mv_matrix, in.proj_matrix, in.viewport,
		   &objx, &objy, &objz)) {
    first = vec3(objx, objy, objz);
  }
  else {
    assert(false);
  }
  if (gluUnProject(static_cast<GLdouble>(last_x),
		   static_cast<GLdouble>(in.viewport[3] - 1 - last_y),
		   winz,
		   in.mv_matrix, in.proj_matrix, in.viewport,
		   &objx, &objy, &objz)) {
    last = vec3(objx, objy, objz);
  }
  else {
    assert(false);
  }
  vec3 translation = last - first;
  for (beziers::iterator p = bs.begin(); p != bs.end(); p++) {
    bezier::ctrl_points::iterator pp = (*p).V.begin();
    bezier::ctrl_points::iterator pp_end = (*p).V.end();
    bezier::curv_centers::iterator pc = (*p).C.begin();
    for (; pp != pp_end; pp++, pc++) {
      (*pp) += translation; // Translate control point
      (*pc) += translation; // Translate curvature center
    }
  }
  // Normal vectors unchanged by translation!
  proba_surface.clear();
  computeBezierSurface();
  computeBoundingBox();
  computeBarycenter();
  clean(in.window);
  buildDisplayLists(in.window);
  initDisplayData();
}

void Stroke3D::addIntersectedStroke(Stroke3D& s) {
    addPStroke(&s);
  s.addPStroke(this);
}

void Stroke3D::addPStroke(Stroke3D* p) {
  pstrokes.push_back(p);
}

void Stroke3D::cleanIntersectedStrokes() {
  for (std::list<Stroke3D*>::const_iterator iter = pstrokes.begin();
       iter != pstrokes.end(); iter++) {
    (*iter)->clean(this);
  }
}

void Stroke3D::clean(Stroke3D* p) {
  for (std::list<Stroke3D*>::iterator iter = pstrokes.begin();
       iter != pstrokes.end(); iter++) {
    if ((*iter) == p) {
      pstrokes.erase(iter);
      return;
    }
  }
}

void Stroke3D::clean(const int window) {
  glutSetWindow(window);
  glDeleteLists(proba_surface_list, 1);
  glDeleteLists(proba_surface_picking_list, 1);
}

void Stroke3D::drawSpline() const {
  glPushAttrib(GL_EVAL_BIT);
  
  beziers::const_iterator p = bs.begin();
  int i = 0;
  for (; p != bs.end(); p++, i++) {
    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, (*p).V.size(), &(*p).V[0][0]);
    const GLint nstep_u = nsteps[i];
    glMapGrid1d(nstep_u, 0.0, 1.0);
    glEvalMesh1(GL_LINE, 0, nstep_u);
  }
  
  glPopAttrib();
}

void Stroke3D::drawOccluder() const {
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(GL_TEXTURE_2D, occluder_tex_name);
  glCallList(proba_surface_list);
  glPopAttrib();
}

void Stroke3D::drawStrokeFirstPass() const {
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(GL_TEXTURE_2D, proba_surface_tex_name);
  drawClippedStroke();
  glPopAttrib();
}

void Stroke3D::drawStrokeSecondPass() const {
  drawClippedStroke();
}

void Stroke3D::drawProbaSurface() const {
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(GL_TEXTURE_2D, proba_surface_tex_name);
  glCallList(proba_surface_list);
  glPopAttrib();
}

void Stroke3D::drawProbaSurfacePicking() const {
  glCallList(proba_surface_picking_list);
}

void Stroke3D::drawIntersectedStrokes() const {
  if (pstrokes.empty()) {
    return;
  }
  glPushAttrib(GL_TRANSFORM_BIT);
  
  glClipPlane(GL_CLIP_PLANE0, &equations[0][0]);
  glClipPlane(GL_CLIP_PLANE1, &equations[1][0]);
  
  std::list<Stroke3D*>::const_iterator iter = pstrokes.begin();
  for (; iter != pstrokes.end(); iter++) {
    if ((*iter)->drawing_mode == TEXTURED_POLYGON) {
	(*iter)->drawProbaSurface();
    }
  }
  
  glPopAttrib();
}

void Stroke3D::drawClippedStroke() const {
  glPushAttrib(GL_TRANSFORM_BIT);
  glClipPlane(GL_CLIP_PLANE0, &equations[0][0]);
  glClipPlane(GL_CLIP_PLANE1, &equations[1][0]);
  glCallList(proba_surface_list);
  glPopAttrib();
}

void Stroke3D::drawControlPoints() const {
  glBegin(GL_POINTS);
  beziers::const_iterator p;
  bezier::ctrl_points::const_iterator cp;
  for (p = bs.begin(); p != bs.end(); p++) {
    for (cp = (*p).V.begin(); cp != (*p).V.end(); cp++) {
      glVertex3d((*cp).x(), (*cp).y(), (*cp).z());
    }
  }
  glEnd();
}

void Stroke3D::drawTangents() const {
  glBegin(GL_LINES);
  beziers::const_iterator p;
  bezier::ctrl_points::const_iterator cp;
  for (p = bs.begin(); p != bs.end(); p++) {
    cp = (*p).V.begin();
    glVertex3d((*cp).x(), (*cp).y(), (*cp).z());
    glVertex3d((*(cp+1)).x(), (*(cp+1)).y(), (*(cp+1)).z());
    cp = (*p).V.end()-1;
    glVertex3d((*cp).x(), (*cp).y(), (*cp).z());
    glVertex3d((*(cp-1)).x(), (*(cp-1)).y(), (*(cp-1)).z());
  }
  glEnd();
}

void Stroke3D::drawCurvatureVectors() const {
  glBegin(GL_LINES);
  beziers::const_iterator p;
  for (p = bs.begin(); p != bs.end(); p++) {
    bezier::ctrl_points::const_iterator cp = (*p).V.begin();
    bezier::curv_centers::const_iterator cc = (*p).C.begin();
    for (; cp != (*p).V.end(); cp++, cc++) {
      glVertex3d((*cp).x(), (*cp).y(), (*cp).z());
      glVertex3d((*cc).x(), (*cc).y(), (*cc).z());
    }
  }
  glEnd();
}

void Stroke3D::drawCircles() const {
  const int nstp = 20; // Magic number!
  const real step_size = (2.0*M_PI)/nstp;
  beziers::const_iterator p;
  for (p = bs.begin(); p != bs.end(); p++) {
    for (int i = 0; i < (*p).C.size(); i++) {
      glBegin(GL_LINE_LOOP);
      for (int t = 0; t < nstp; t++) {
	vec3 M = (*p).C[i] +
	  ((*p).R[i])*(Numerics<real>::cosine(t*step_size)*(*p).N[i] +
		       Numerics<real>::sine(t*step_size)*plane_normal);
	glVertex3d(M.x(), M.y(), M.z());
      }
      glEnd();
    }
  }
}

void Stroke3D::drawNormals() const {
  glBegin(GL_LINES);
  beziers::const_iterator p;
  for (p = bs.begin(); p != bs.end(); p++) {
    bezier::ctrl_points::const_iterator cp = (*p).V.begin();
    bezier::normals::const_iterator n = (*p).N.begin();
    for (; cp != (*p).V.end(); cp++, n++) {
      glVertex3d((*cp).x(), (*cp).y(), (*cp).z());
      glVertex3d((*cp).x()+(*n).x(), (*cp).y()+(*n).y(), (*cp).z()+(*n).z());
    }
  }
  glEnd();
}

void Stroke3D::drawBarycenter() const {
  glBegin(GL_POINTS);
  glVertex3dv(&barycenter_global[0]);
  glEnd();
}

void Stroke3D::drawBoundingBox() const {
  glColor4fv(color);
  box.draw();
}
