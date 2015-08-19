#include "drawing.h"

using namespace std;

void Drawing::setStrokesColor(const GLfloat color[4]) {
  strokes::iterator p_end = strks.end();
  for (strokes::iterator p = strks.begin(); p != p_end; p++) {
    (*p).setInitColor(color);
  }
}

void Drawing::addReadStroke(const stroke& s) {
  if (!s.empty()) {
    strks.push_back(s);
    strokes::iterator p_last = --strks.end();
    (*p_last).occluder_tex_name      = occluder_tex_name;
    (*p_last).proba_surface_tex_name = proba_surface_tex_name;
    (*p_last).stroke_tex_name        = stroke_tex_name;
    // No texture init... In the future!
    // No color init!
    for (strokes::iterator p = strks.begin(); p != p_last; p++) {
      if ((*p_last).box.isIntersectedBy((*p).box)) {
	(*p_last).addIntersectedStroke(*p);
      }
    }
  }
}

Drawing::Drawing()
  : background_tex_name(0),
    occluder_tex_name(0), proba_surface_tex_name(0), stroke_tex_name(0),
    accumulation(false) {
  texs.reserve(5);   // Magic number!
  const int n = 20;
  const GLfloat background_array[n] = { 0.0,  0.0, 0.0, 0.0, 0.0,
				       10.0,  0.0, 1.0, 0.0, 0.0,
				       10.0, 10.0, 1.0, 1.0, 0.0,
                                        0.0, 10.0, 0.0, 1.0, 0.0};
  background = std::vector<GLfloat>(background_array, background_array + n);
  transparent_plane = std::vector<GLfloat>(12, 0.0);
  p_selected_stroke_prev = strks.end();
}

void Drawing::setColor(const GLfloat color[4], const colortype type) {
  if (type == BACKGROUND_COLOR) {
    background_color[0] = color[0]; background_color[1] = color[1];
    background_color[2] = color[2]; background_color[3] = color[3];
  }
  else if (type == STROKE_COLOR) {
    stroke_color[0] = color[0]; stroke_color[1] = color[1];
    stroke_color[2] = color[2]; stroke_color[3] = color[3];
    setStrokesColor(stroke_color);
  }
  else if (type == SELECTED_STROKE_COLOR) {
    selected_stroke_color[0] = color[0]; selected_stroke_color[1] = color[1];
    selected_stroke_color[2] = color[2]; selected_stroke_color[3] = color[3];
  }
  else {
    assert(false);
  }
}

void Drawing::addTexture(const Texture& tex, const textype type) {
  texs.push_back(tex);
  if (type == BACKGROUND) {
    background_tex_name = tex.name;
  }
  else if (type == OCCLUDER) {
    occluder_tex_name = tex.name;
  }
  else if (type == PROBA_SURFACE) {
    proba_surface_tex_name = tex.name;
  }
  else if (type == STROKE) {
    stroke_tex_name = tex.name;
  }
  else {
    assert(false);
  }
}

void Drawing::addStroke(const stroke& s) {
  if (!s.empty()) {
    strks.push_back(s);
    strokes::iterator p_last = --strks.end();
    (*p_last).occluder_tex_name      = occluder_tex_name;
    (*p_last).proba_surface_tex_name = proba_surface_tex_name;
    (*p_last).stroke_tex_name        = stroke_tex_name;
    (*p_last).setInitColor(stroke_color);
    for (strokes::iterator p = strks.begin(); p != p_last; p++) {
      if ((*p_last).box.isIntersectedBy((*p).box)) {
	(*p_last).addIntersectedStroke(*p);
      }
    }
  }
}
/* TODO:
   Naive implementation: should use clustering and test intersection with
   an AABB_Tree instead.
*/

void Drawing::markStroke(const Input& in) {
  strokes::iterator p_selected_stroke_curr = strks.begin();
  std::advance(p_selected_stroke_curr, in.selectedStrokeID());
  if (p_selected_stroke_curr == p_selected_stroke_prev) {
    (*p_selected_stroke_curr).reinitColor();
    p_selected_stroke_prev = strks.end();
  }
  else {
    (*p_selected_stroke_curr).setColor(selected_stroke_color);
    if (p_selected_stroke_prev != strks.end()) {
      (*p_selected_stroke_prev).reinitColor();
    }
    p_selected_stroke_prev = p_selected_stroke_curr;
  }
}

void Drawing::unmarkStroke() {
  if (p_selected_stroke_prev != strks.end()) {
    (*p_selected_stroke_prev).reinitColor();
    p_selected_stroke_prev = strks.end();
  }
}

void Drawing::startMovingStroke(int x, int y) {
  if (p_selected_stroke_prev != strks.end()) {
    first_x = x;
    first_y = y;
  }
}

void Drawing::stopMovingStroke(int x, int y, const Input& in) {
  if (p_selected_stroke_prev != strks.end()) {
    last_x = x;
    last_y = y;
    (*p_selected_stroke_prev).translate(first_x, first_y, last_x, last_y, in);
  }
}

void Drawing::reverseStroke(const Input& in) {
  if (p_selected_stroke_prev == strks.end()) {
    strks.back().reverse(in.window);
  }
  else {
    (*p_selected_stroke_prev).reverse(in.window);
  }
}

void Drawing::removeStroke(const Input& in) {
  if (!strks.empty()) {
    if (p_selected_stroke_prev == strks.end()) {
      strks.back().clean(in.window);
      strks.back().cleanIntersectedStrokes();
      strks.pop_back();
    }
    else {
      (*p_selected_stroke_prev).clean(in.window);
      (*p_selected_stroke_prev).cleanIntersectedStrokes();
      strks.erase(p_selected_stroke_prev);
      p_selected_stroke_prev = strks.end();
    }
  }
}

void Drawing::clearStrokes(const Input& in) {
  const int window = in.window;
  for (strokes::iterator p = strks.begin(); p != strks.end(); p++) {
    (*p).clean(window);
  }
  strks.clear();
}

void Drawing::setBackgroundVertices(const Input& in) {
  static const GLint win[4][2] = {{0, 1}, {2, 1}, {2, 3}, {0, 3}};
  for (int i = 0; i < 4; i++) {
    GLdouble objx, objy, objz;
    if (gluUnProject(in.viewport[win[i][0]], in.viewport[win[i][1]],
		     0.2, /* Magic number! */
		     in.mv_matrix, in.proj_matrix, in.viewport,
		     &objx, &objy, &objz)) {
      background[2 + 5*i    ] = static_cast<GLfloat>(objx);
      background[2 + 5*i + 1] = static_cast<GLfloat>(objy);
      background[2 + 5*i + 2] = static_cast<GLfloat>(objz);
    }
    else {
      assert(false);
    }
  }
}

void Drawing::setTransparentPlaneVertices(const Input& in) {
  static const GLint win[4][2] = {{0, 1}, {2, 1}, {2, 3}, {0, 3}};
  const GLdouble winz = 0.5*(in.getFirstPlane() + in.getLastPlane());
  const GLdouble ratio = 0.05;
  const GLdouble x_ratio = ratio*in.viewport[2];
  const GLdouble y_ratio = ratio*in.viewport[3];
  const GLdouble viewport_mod[4]
    = {in.viewport[0] + x_ratio, in.viewport[1] + y_ratio,
       in.viewport[2] - x_ratio, in.viewport[3] - y_ratio};
  for (int i = 0; i < 4; i++) {
    GLdouble objx, objy, objz;
    if (gluUnProject(viewport_mod[win[i][0]], viewport_mod[win[i][1]],
		     winz,
		     in.mv_matrix, in.proj_matrix, in.viewport,
		     &objx, &objy, &objz)) {
      transparent_plane[3*i    ] = static_cast<GLfloat>(objx);
      transparent_plane[3*i + 1] = static_cast<GLfloat>(objy);
      transparent_plane[3*i + 2] = static_cast<GLfloat>(objz);
    }
    else {
      assert(false);
    }
  }
}

/* TODO:
   . ecrire une fonction gluUnProject qui ne reinverse pas systematiquement la
   matrice -> gain de temps!
   . incorporer a une class OpenGLWindow ?
*/

void Drawing::setAccumulationMode(const bool choice) {
  accumulation = choice;
}

void Drawing::paintBackground() const {
  glPushAttrib(GL_TEXTURE_BIT);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, background_tex_name);
  glInterleavedArrays(GL_T2F_V3F, 0, &background[0]);
  glDrawArrays(GL_QUADS, 0, 4);
  
  glPopClientAttrib();
  glPopAttrib();
}

void Drawing::paintTransparentPlane() const {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT |
	       GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnableClientState(GL_VERTEX_ARRAY);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glVertexPointer(3, GL_FLOAT, 0, &transparent_plane[0]);
  
  glColor4f(0.75, 0.75, 0.75, 0.25);
  glDrawArrays(GL_QUADS, 0, 4);
  
  glPopClientAttrib();
  glPopAttrib();
}

bool Drawing::read(const char* name, const int window) {
  ifstream file_in(name);
  if (!file_in) {
    return false;
  }
  char line[256];
  file_in.getline(line, 256, '\n');
  int n;
  sscanf(line, "%d", &n);
  for (int i = 0; i < n; i++) {
    stroke s;
    s.read(file_in, window);
    addReadStroke(s);
  }
  file_in.close();
  return true;
}

bool Drawing::readOneByOne(const char* name, const int window) {
  static ifstream file_in;
  static int n;
  static int i = 0;
  static bool first_time = true;
  if (first_time) {
    file_in.open(name);
    if (!file_in) {
      return false;
    }
    char line[256];
    file_in.getline(line, 256, '\n');
    sscanf(line, "%d", &n);
    first_time = false;
  }
  if (i < n) {
    stroke s;
    s.read(file_in, window);
    addReadStroke(s);
    i++;
    return true;
  }
  else {
    file_in.close();
    i = 0;
    first_time = true;
    return false;
  }
}

bool Drawing::write(const char* name) const {
  ofstream file_out(name, ios::out);
  if (!file_out) {
    return false;
  }
  file_out << strks.size() << endl;
  strokes::const_iterator p;
  for (p = strks.begin(); p != strks.end(); p++) {
    (*p).write(file_out);
  }
  file_out.close();
  return true;
}

void Drawing::drawSelection() const {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_HINT_BIT |
	       GL_LINE_BIT | GL_POLYGON_BIT);
  
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_3);
  glLineWidth(line_width);
  
  GLuint name = 0;
  for (strokes::const_iterator s = strks.begin(); s != strks.end(); s++) {
    glLoadName(name);
    name++;
    (*s).drawProbaSurfacePicking();
  }
  
  glPopAttrib();
}

void Drawing::drawFeedback(const Input& in) const {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_HINT_BIT |
	       GL_LINE_BIT | GL_POLYGON_BIT);
  
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_3);
  glLineWidth(line_width);
  
#if 0
  GLfloat stroke_number_token = 0.0;
  const GLfloat proba_surface_token
    = in.object_tokens[Input::PROBA_SURFACE_INDEX];
  for (strokes::iterator s = strks.begin(); s != strks.end(); s++) {
    glPassThrough(stroke_number_token);
    stroke_number_token += 1.0;
    glPassThrough(proba_surface_token);
    (*s).drawProbaSurfacePicking();
  }
#else
  glPassThrough(in.object_tokens[Input::PROBA_SURFACE_INDEX]);
  for (strokes::const_iterator s = strks.begin(); s != strks.end(); s++) {
    (*s).drawProbaSurfacePicking();
  }
#endif
  
  glPopAttrib();
}

void Drawing::drawInformations(const Input& in) {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_HINT_BIT |
	       GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT |
	       GL_POINT_BIT | GL_LINE_BIT | GL_POLYGON_BIT);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CLIP_PLANE0);
  glEnable(GL_CLIP_PLANE1);
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_3);
#if ANTIALIASING
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  for (strokes::iterator s = strks.begin(); s != strks.end(); s++) {
    //glColor3f(0.0, 1.0, 1.0);
    //(*s).drawCurvatureVectors();
    //glColor3f(0.0, 0.0, 1.0);
    //(*s).drawCircles();
    //glColor3f(1.0, 0.0, 0.0);
    //(*s).drawControlPoints();
    //glColor3f(0.0, 1.0, 0.0);
    //(*s).drawTangents();
    //glColor3f(0.0, 1.0, 1.0);
    //(*s).drawNormals();
    
    glLineWidth(1.0);
    glColor4fv((*s).color);
    (*s).drawProbaSurface();
    
    if ((*s).drawing_mode == Stroke3D::TEXTURED_POLYGON) {
      (*s).move(in);
      //(*s).drawBoundingBox();
      //glPointSize(5.0);
      //glColor3f(1.0, 0.0, 0.0);
      //(*s).drawBarycenter();
      glLineWidth(line_width);
      (*s).drawClippedStroke();
      if (accumulation) {
	(*s).drawIntersectedStrokes();
      }
    }
  }
  
  glPopAttrib();
}

void Drawing::draw(const Input& in) {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_HINT_BIT |
	       GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	       GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT |
	       GL_POINT_BIT | GL_LINE_BIT | GL_POLYGON_BIT);
  
  glEnable(GL_CLIP_PLANE0);
  glEnable(GL_CLIP_PLANE1);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_3);
  glEnable(GL_MAP2_TEXTURE_COORD_2);
#if ANTIALIASING
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  
  glDepthMask(GL_FALSE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
  glAlphaFunc(GL_NOTEQUAL, 0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(line_width);
  
  /* Draw strokes */
  for (strokes::iterator s = strks.begin(); s != strks.end(); s++) {
    if ((*s).drawing_mode == Stroke3D::LINE) {
      glPushAttrib(GL_DEPTH_BUFFER_BIT);
      glDepthMask(GL_TRUE);
      glColor4fv((*s).color);
      (*s).drawSpline();
      glPopAttrib();
    }
    else if ((*s).drawing_mode == Stroke3D::TEXTURED_POLYGON) {
      (*s).move(in);
      glColor4fv((*s).color);
      (*s).drawStrokeFirstPass();
    }
  }
  
  glEnable(GL_POLYGON_OFFSET_FILL);
  
  glDepthMask(GL_TRUE);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glAlphaFunc(GL_GREATER, 0.05); // Magic number!
  glPolygonOffset(1.0, 1.0);
  
  /* Draw occluders in depth buffer */
  for (strokes::const_iterator s = strks.begin(); s != strks.end(); s++) {
    if ((*s).drawing_mode == Stroke3D::TEXTURED_POLYGON ||
	(*s).drawing_mode == Stroke3D::OCCLUSION) {
      (*s).drawOccluder();
    }
  }
  
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  glStencilMask(0x00000001);
  glDepthMask(GL_FALSE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
  glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);
  glColor4fv(background_color);
  
  /* Draw occluders in color buffer */
  for (strokes::const_iterator s = strks.begin(); s != strks.end(); s++) {
    if ((*s).drawing_mode == Stroke3D::TEXTURED_POLYGON) {
      
      /* Avoid drawing over its own stroke */
      glPushAttrib(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glEnable(GL_STENCIL_TEST);
      
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glStencilFunc(GL_EQUAL, 0x00000000, 0x00000001);
      (*s).drawStrokeSecondPass();
      
      glStencilMask(0x00000000);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
      (*s).drawOccluder();
      
      glStencilMask(0x00000001);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glStencilFunc(GL_EQUAL, 0x00000001, 0x00000001);
      (*s).drawStrokeSecondPass();
      
      glPopAttrib();
    }
    else if ((*s).drawing_mode == Stroke3D::OCCLUSION) {
      (*s).drawOccluder();
    }
  }
  
  glPopAttrib();
}
