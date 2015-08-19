#include "input.h"

using namespace std;

const Input::real Input::npixels_min = 3.0;
const GLfloat Input::MAX_WINZ = 1.0;
const GLuint Input::DEFAULT_NAME = ~0;

Input::Input()
  : local_plane(false),
    window_z_global(0.0), window_z_offset_global(0.0),//useful?
    window(0),
    window_z_first(0.0), window_z_offset_first(0.0),
    window_z_last(0.0), window_z_offset_last(0.0) {
  const int n = 100; // Magic number!
  vertices.reserve(n);
  positions.reserve(n);
  object_tokens = std::vector<GLfloat>(2);
  object_tokens[SCENE_INDEX]         = SCENE_TOKEN;
  object_tokens[PROBA_SURFACE_INDEX] = PROBA_SURFACE_TOKEN;
}

void Input::setViewVector() {
  writeViewVector(mv_matrix, view_vector);
}

void Input::setPointColor(const GLfloat color[4]) {
  point_color[0] = color[0]; point_color[1] = color[1];
  point_color[2] = color[2]; point_color[3] = color[3];
}

void Input::setLocalPlaneMode(bool choice) {
  local_plane = choice;
}

void Input::setGlobalPlane() {
  static const GLdouble origin[3] = {0.0, 0.0, 0.0};
  GLdouble winx, winy, winz;
  if (gluProject(origin[0], origin[1], origin[2],
		 mv_matrix, proj_matrix, viewport,
		 &winx, &winy, &winz)) {
    if (winz > 1.0) {
      window_z_global = 1.0;
    }
    else if (winz < 0.0) {
      window_z_global = 0.0;
    }
    else {
      window_z_global = winz;
    }
    window_z_first = window_z_global;
    window_z_last  = window_z_global;
  }
  else {
    assert(false);
  }
}

void Input::setGlobalPlaneOffset(GLdouble offset) {
  if (offset > 0.0) {
    GLdouble window_z_remaining = 1.0 - window_z_global;
    if (offset > window_z_remaining) {
      window_z_offset_global = window_z_remaining;
    }
    else {
      window_z_offset_global = offset;
    }
  }
  else if (offset < 0.0) {
    GLdouble window_z_global_neg = - window_z_global;
    if (offset < window_z_global_neg) {
      window_z_offset_global = window_z_global_neg;
    }
    else {
      window_z_offset_global = offset;
    }
  }
  else {
    window_z_offset_global = 0.0;
  }
  window_z_offset_first = window_z_offset_global;
  window_z_offset_last  = window_z_offset_global;
}

/* Prints out the contents of the selection array */
void printSelectionBuffer(const GLint size, const GLuint* buffer) {
  GLuint names;
  GLuint* ptr = const_cast<GLuint*>(buffer);
  printf("\nNumber of hits: %d\n", size);
  for (GLint i = 0; i < size; i++) { /* For each hit */
    names = *ptr;
    printf("Number of names for hit %d: %d\n", i, names); ptr++;
    printf("z min is %u; ", *ptr); ptr++;
    printf("z max is %u\n", *ptr); ptr++;
    (names > 1) ? printf("The names are ") : printf("The name is ");
    for (int j = 0; j < names; j++) { /* For each name */
      printf("%d ", *ptr); ptr++;
    }
    printf("\n");
  }
}
/* TODO:
   . Convert z values to floating point number.
*/

/* Write contents of one vertex to stdout. */
void print3DVertex(const GLint size, GLint* count, const GLfloat* buffer) {
  printf("  ");
  for (GLint i = 0; i < 3; i++) {
    printf("%4.6f ", buffer[size-(*count)]);
    *count = *count - 1;
  }
  printf ("\n");
}

/* Write contents of entire buffer. (Parse tokens!) */
void printFeedbackBuffer(const GLint size, const GLfloat* buffer) {
  GLint count;
  GLfloat token;
  count = size;
  while (count) {
    token = buffer[size-count];
    count--;
    if (token == GL_PASS_THROUGH_TOKEN) {
      printf("GL_PASS_THROUGH_TOKEN\n");
      printf("  %4.6f\n", buffer[size-count]);
      count--;
    }
    else if (token == GL_LINE_TOKEN) {
      printf("GL_LINE_TOKEN\n");
      print3DVertex(size, &count, buffer);
      print3DVertex(size, &count, buffer);
    }
    else if (token == GL_POLYGON_TOKEN) {
      printf("GL_POLYGON_TOKEN\n");
      GLfloat n = buffer[size-count];
      count--;
      printf("  %4.6f\n", n);
      for (GLint i = 0; i < static_cast<GLint>(n); i++) {
	print3DVertex(size, &count, buffer);
      }
    }
  }
}

GLuint Input::parseSelectionBuffer(const GLint size, const GLuint* buffer) {
  GLuint winz_min = DEFAULT_NAME; // Maximum unsigned int value
  GLuint name = DEFAULT_NAME;
  GLuint* ptr = const_cast<GLuint*>(buffer);
  for (GLuint i = 0; i < size; i++) { // For each hit
    GLuint names = *ptr;
    ptr++;
    GLuint winz_min_curr = *ptr;
    ptr++; ptr++;
    GLuint name_curr = DEFAULT_NAME;
    for (GLuint j = 0; j < names; j++) { // For each name
      name_curr = *ptr;
      ptr++;
    }
    if (winz_min_curr < winz_min) {
      winz_min = winz_min_curr;
      name = name_curr;
    }
  }
  return name;
}

void Input::parseFeedbackBuffer(const GLint size, const GLfloat* buffer) {
  GLint count;
  GLfloat token, object_token, object_number_token;
  count = size;
  while (count) {
    token = buffer[size-count];
    count--;
    if (token == GL_PASS_THROUGH_TOKEN) {
      GLfloat pass_through_token = buffer[size-count];
      if (pass_through_token < 0.0) {
	object_token = pass_through_token;
      }
      else {
	object_number_token = pass_through_token;
      }
      count--;
    }
    else if (token == GL_LINE_TOKEN) {
      GLfloat mean_winz = 0.0;
      for (GLint i = 0; i < 2; i++) {
	count -= 2;
	mean_winz += buffer[size-count];
	count--;
      }
      mean_winz *= 0.5;
      if (mean_winz < results[object_token][OBJECT_WINZ]) {
	results[object_token][OBJECT_NUMBER] = object_number_token;
	results[object_token][OBJECT_WINZ]   = mean_winz;
      }
    }
    else if (token == GL_POLYGON_TOKEN) {
      GLint n = static_cast<GLint>(buffer[size-count]);
      count--;
      GLfloat mean_winz = 0.0;
      for (GLint i = 0; i < n; i++) {
	count -= 2;
	mean_winz += buffer[size-count];
	count--;
      }
      mean_winz /= n;
      if (mean_winz < results[object_token][OBJECT_WINZ]) {
	results[object_token][OBJECT_NUMBER] = object_number_token;
	results[object_token][OBJECT_WINZ]   = mean_winz;
      }
    }
  }
}

void Input::processFeedbackResults(const GLint size, const GLfloat* buffer) {
  /* Init results map */
  const GLfloat default_number = 0.0; // Default object number
  for (int i = SCENE_INDEX; i < PROBA_SURFACE_INDEX + 1; i++) {
    results[object_tokens[i]] = Vec2<GLfloat>(default_number, MAX_WINZ);
  }
  
  /* Parse */
  parseFeedbackBuffer(size, buffer);
#if DEBUG
  printFeedbackBuffer(size, buffer);
  cerr << "Results:" << endl;
  std::map< GLfloat, Vec2<GLfloat> >::const_iterator pm;
  for (pm = results.begin(); pm != results.end(); pm++) {
    cerr << (*pm).first << " " << (*pm).second << endl;
  }
#endif
  
  /* Analyze */
  nearer_token = SCENE_TOKEN;
  nearer_winz  = MAX_WINZ;
  std::map< GLfloat, Vec2<GLfloat> >::const_iterator p;
  for (p = results.begin(); p != results.end(); p++) {
    const GLfloat winz = (*p).second[OBJECT_WINZ];
    if (winz < MAX_WINZ) {
      nearer_token = (*p).first;
      nearer_winz = winz;
    }
  }
}

bool Input::selectStroke(const GLint size, const GLuint* buffer) {
  bool is_selected;
  if (size == 0) {
    is_selected = false;
  }
  else {
    selected_name = parseSelectionBuffer(size, buffer);
#if DEBUG
    printSelectionBuffer(size, buffer);
    cerr << "Selected name: " << selected_name << endl << endl;
#endif
    if (selected_name == DEFAULT_NAME) {
      is_selected = false;
    }
    else {
      is_selected = true;
    }
  }
  return is_selected;
}

void Input::setPlanes(const GLint size_first, const GLfloat* buffer_first,
		      const GLint size_last, const GLfloat* buffer_last) {
  if (local_plane) {
    if (size_first == 0) {
      // Stay at the same depth as previous stroke (if trackball doesn't move!)
      projection_mode = FOLLOW;
    }
    else {
      processFeedbackResults(size_first, buffer_first);
      
      /* Determine projection plane */
      if (nearer_winz == MAX_WINZ) {
	// Idem
	projection_mode = FOLLOW;
      }
      else {
	if (nearer_token == SCENE_TOKEN) {
	  window_z_first        = nearer_winz;
	  window_z_offset_first = 0.0;
	  projection_mode = SPLAT;
	}
        else if (nearer_token == PROBA_SURFACE_TOKEN) {
	  window_z_first        = nearer_winz;
	  window_z_offset_first = 0.0;
	  projection_mode = SPLAT;
	}
      }
    }
    if (size_last == 0) {
      window_z_last        = window_z_first;
      window_z_offset_last = window_z_offset_first;
    }
    else {
      processFeedbackResults(size_last, buffer_last);
      
      /* Determine projection plane */
      if (nearer_winz == MAX_WINZ) {
	window_z_last        = window_z_first;
        window_z_offset_last = window_z_offset_first;
      }
      else {
	if (nearer_token == SCENE_TOKEN) {
	  window_z_last        = nearer_winz;
	  window_z_offset_last = 0.0;
	  projection_mode = BRIDGE;
	}
        else if (nearer_token == PROBA_SURFACE_TOKEN) {
	  window_z_last        = nearer_winz;
	  window_z_offset_last = 0.0;
	  projection_mode = BRIDGE;
	}
      }
    }
  }
  else {
    window_z_first        = window_z_global;
    window_z_offset_first = window_z_offset_global;
    window_z_last         = window_z_first;
    window_z_offset_last  = window_z_offset_first;
    projection_mode = FOLLOW;
  }
#if DEBUG
  if (projection_mode == FOLLOW) {
    cerr << "FOLLOW" << endl;
  }
  else if (projection_mode == SPLAT) {
    cerr << "SPLAT" << endl;
  }
  else if (projection_mode == BRIDGE) {
    cerr << "BRIDGE" << endl;
  }
  else {
    assert(false);
  }
  cerr << endl;
#endif
}

bool Input::addPoint2D(const GLint x, const GLint y) {
  bool added;
  vec2 p(x, y);
  if (positions.empty()) {
    positions.push_back(p);
    added = true;
  }
  else {
    // Remove coincident or nearly coincident data points
    if (dist(p, positions.back().pos) > npixels_min) {
      positions.push_back(p);
      added = true;
    }
    else {
      added = false;
    }
  }
  return added;
}

void Input::addPoint(const GLint x, const GLint y) {
  if (addPoint2D(x, y)) {
    GLdouble objx, objy, objz;
    if (gluUnProject(static_cast<GLdouble>(x),
		     static_cast<GLdouble>(viewport[3] - 1 - y),
		     0.05, /* Magic number! */
		     mv_matrix, proj_matrix, viewport,
		     &objx, &objy, &objz)) {
      vertices.push_back(vec3(objx, objy, objz));
    }
    else {
      assert(false);
    }
  }
}

GLdouble Input::getFirstPlane() const {
  return window_z_first + window_z_offset_first;
}

GLdouble Input::getLastPlane() const {
  return window_z_last + window_z_offset_last;
}

GLuint Input::selectedStrokeID() const {
  return selected_name;
}

int Input::projectionMode() const {
  return projection_mode;
}

bool Input::read(const char* name) {
  ifstream file_in(name);
  if (!file_in) {
    return false;
  }
  int n;
  real x, y;
  char line[256];
  file_in.getline(line, 256, '\n');
  sscanf(line, "%d", &n);
  for (int i = 0; i < n; i++) {
    file_in.getline(line, 256, '\n');
    sscanf(line, "%lf %lf", &x, &y);
    positions.push_back(point(vec2(x, y)));
  }
  file_in.close();
  return true;
}

bool Input::write(const char* name) const {
  ofstream file_out(name);
  if (!file_out) {
    return false;
  }
  file_out << positions.size() << endl;
  std::vector<point>::const_iterator p;
  for (p = positions.begin(); p != positions.end(); p++) {
    file_out << (*p).pos << endl;
  }
  file_out.close();
  return true;
}

void Input::fair() {
  const real k = 1.0/16.0; // Magic number!
  std::vector<point> tmp(positions);
  std::vector<point>::const_iterator tp = tmp.begin()+1;
  std::vector<point>::iterator p = positions.begin()+1;
  for (; tp != tmp.end()-1; tp++, p++) {
    (*p).pos = k*(*(tp-1)).pos + (1.0 - 2.0*k)*(*tp).pos + k*(*(tp+1)).pos;
  }
}

void Input::draw2D() const {
  glPushAttrib(GL_POINT_BIT | GL_CURRENT_BIT);
  glPointSize(point_size);
  glColor4fv(point_color);
  glBegin(GL_POINTS);
  std::vector<point>::const_iterator p;
  for (p = positions.begin(); p != positions.end(); p++) {
    glVertex2dv(&(*p).pos[0]);
  }
  glEnd();
  glPopAttrib();
}

void Input::draw() const {
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
  glEnable(GL_DEPTH_TEST);
  glPointSize(point_size);
  glColor4fv(point_color);
  glBegin(GL_POINTS);
  std::vector<vec3>::const_iterator p;
  for (p = vertices.begin(); p != vertices.end(); p++) {
    glVertex3dv(&(*p)[0]);
  }
  glEnd();
  glPopAttrib();
}

void Input::clear() {
  positions.clear();
  vertices.clear();
}
