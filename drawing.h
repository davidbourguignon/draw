#ifndef DRAWING_H
#define DRAWING_H

#include "trackball.h"
#include "stroke3D.h"
#include "texture.h"

class Drawing {
private:
  typedef GLdouble             real;
  typedef Vec3<real>           vec3;
  typedef Quat<real>           quat;
  typedef Trackball<real>      trackball;
  typedef Stroke3D             stroke;
  typedef std::list<stroke>    strokes;
  typedef std::vector<Texture> textures;
  
  void setStrokesColor(const GLfloat color[4]);
  void addReadStroke(const stroke& s);
  
  textures texs;
  strokes strks;
  
  GLuint background_tex_name;
  GLuint occluder_tex_name;
  GLuint proba_surface_tex_name;
  GLuint stroke_tex_name;
  
  std::vector<GLfloat> background;        // Background array
  std::vector<GLfloat> transparent_plane; // Transparent plane array
  GLfloat stroke_color[4];
  GLfloat selected_stroke_color[4];
  GLfloat background_color[4];
  
  bool accumulation;
  strokes::iterator p_selected_stroke_prev;
  int first_x, first_y, last_x, last_y;
  
public:
  enum textype {BACKGROUND, OCCLUDER, PROBA_SURFACE, STROKE};
  enum colortype {BACKGROUND_COLOR, STROKE_COLOR, SELECTED_STROKE_COLOR};
  
  Drawing();
  void setColor(const GLfloat color[4], const colortype type);
  void addTexture(const Texture& tex, const textype type);
  void addStroke(const stroke& s);
  void markStroke(const Input& in);
  void unmarkStroke();
  void startMovingStroke(int x, int y);
  void stopMovingStroke(int x, int y, const Input& in);
  void reverseStroke(const Input& in);
  void removeStroke(const Input& in);
  void clearStrokes(const Input& in);
  void setBackgroundVertices(const Input& in);
  void setTransparentPlaneVertices(const Input& in);
  void setAccumulationMode(const bool choice = true);
  bool read(const char* name, const int window);
  bool readOneByOne(const char* name, const int window);
  bool write(const char* name) const;
  void paintBackground() const;
  void paintTransparentPlane() const;
  void drawSelection() const;
  void drawFeedback(const Input& in) const;
  void drawInformations(const Input& in);
  void draw(const Input& in);
  
  GLfloat point_size;
  GLfloat line_width;
};

#endif // DRAWING_H
