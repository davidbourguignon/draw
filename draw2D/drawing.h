#ifndef DRAWING_H
#define DRAWING_H

#include "stroke2D.h"

class Drawing {
  typedef double   real;
  typedef Stroke2D stroke;
  
  GLfloat bg_color[4];
  GLint viewport[4];
  GLdouble frustum_width;
  GLdouble frustum_height;
  GLdouble pixel_size;
  int mouse_mode;
  
  std::vector<stroke> strokes;
  
public:
  Drawing();
  void setBgColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
  void setViewport(const GLint* v);
  void setFrustumSize(GLdouble fw, GLdouble fh);
  void setMouseMode(int m);
  GLint getViewport(int n) const;
  GLdouble pixelSize() const;
  int mouseMode() const;
  void addStroke(const Stroke2D& s);
  void read(const char* name);        // Useful for debugging
  void write(const char* name) const; //
  void draw();
};

#endif // DRAWING_H
