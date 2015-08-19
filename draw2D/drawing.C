#include "drawing.h"

using namespace std;

Drawing::Drawing() {
  strokes.reserve(1000); // Magic number!
}

void Drawing::setBgColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  bg_color[0] = r; bg_color[1] = g; bg_color[2] = b; bg_color[3] = a;
  glClearColor(r, g, b, a);
}

void Drawing::setViewport(const GLint* v) {
  viewport[0] = v[0]; viewport[1] = v[1];
  viewport[2] = v[2]; viewport[3] = v[3]; 
}

void Drawing::setFrustumSize(GLdouble fw, GLdouble fh) {
  frustum_width = fw;
  frustum_height = fh;
  
  if (viewport[2] != 0 && viewport[3] != 0) {
    GLdouble pixel_width = frustum_width/viewport[2];
    GLdouble pixel_height = frustum_height/viewport[3];
    pixel_size = 0.5*(pixel_width + pixel_height);
  }
  else
    cerr << "Error: viewport values missing!" << endl;
}

void Drawing::setMouseMode(int m) {
  mouse_mode = m;
}

GLint Drawing::getViewport(int n) const {
  return viewport[n];
}

GLdouble Drawing::pixelSize() const {
  return pixel_size;
}

int Drawing::mouseMode() const {
  return mouse_mode;
}

void Drawing::addStroke(const Stroke2D& s) {
  if (!s.empty())
    strokes.push_back(s);
}

void Drawing::read(const char* name) {
  ifstream file_in(name, ios::in);
  if (!file_in) {
    cerr << "Error: cannot open input file " << name << endl;
    assert(false);
  }
  char line[256];
  file_in.getline(line, 256, '\n');
  int n;
  sscanf(line, "%d", &n);
  for (int i = 0; i < n; i++) {
    stroke s;
    s.read(file_in);
    addStroke(s);
  }
  file_in.close();
}

void Drawing::write(const char* name) const {
  ofstream file_out(name, ios::out);
  if (!file_out) {
    cerr << "Error: cannot open output file " << name << endl;
    assert(false);
  }
  file_out << strokes.size() << endl;
  std::vector<stroke>::const_iterator p;
  for (p = strokes.begin(); p != strokes.end(); p++)
    (*p).write(file_out);
  file_out.close();
}

void Drawing::draw() {
  std::vector<Stroke2D>::iterator s;
  for (s = strokes.begin(); s != strokes.end(); s++) {
    (*s).drawDerivatives();
    glColor3f(1.0,0.0,0.0);
    (*s).drawControlPoints();
    glColor3f(0.0,1.0,0.0);
    (*s).drawTangents();
    glColor3f(0.0,0.0,0.0);
    (*s).drawSpline();
    glColor3f(0.0,0.0,0.0);
#if TEST_STROKE2D
    (*s).evalCurvatureCenters();
#endif
    glColor3f(0.0,0.0,1.0);
    (*s).drawCurvatureVectors();
  }
}
