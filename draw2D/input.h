#ifndef INPUT_H
#define INPUT_H

#include <fstream>
#include <vector>
#include <map>
#include <GL/glut.h>
#include "vec3.h"
#include "opengl_utils.h"
#include "point.h"

class Input {
private:
  typedef GLdouble          real;
  typedef Vec2<real>        vec2;
  typedef Point<real, vec2> point;
  typedef Vec3<real>        vec3;
  enum objecttokens {SCENE_TOKEN = -2, PROBA_SURFACE_TOKEN = -1};
  enum objectdata   {OBJECT_NUMBER, OBJECT_WINZ};
  
  void parseFeedbackBuffer(const GLint size, const GLfloat* buffer);
  void processFeedbackResults(const GLint size, const GLfloat* buffer);
  GLuint parseSelectionBuffer(const GLint size, const GLuint* buffer);
  
  std::vector<vec3> vertices;
  bool local_plane;
  GLdouble window_z_global;
  GLdouble window_z_offset_global;
  GLdouble window_z_first, window_z_last;
  GLdouble window_z_offset_first, window_z_offset_last;
  int projection_mode;
  GLfloat point_color[4];
  
  std::map< GLfloat, Vec2<GLfloat> > results; // Feedback results
  GLfloat nearer_token, nearer_winz;
  GLuint selected_name;                       // Selection results
  
  // Minimum distance between positions (in pixels)
  static const real npixels_min;
  static const GLfloat MAX_WINZ; // Default depth
  
public:
  enum objectindices {SCENE_INDEX, PROBA_SURFACE_INDEX};
  enum projectionmode {FOLLOW, SPLAT, BRIDGE};
  
  Input();
  void setViewVector();
  void setPointColor(const GLfloat color[4]);
  void setLocalPlaneMode(bool choice = true);
  void setGlobalPlane();
  void setGlobalPlaneOffset(GLdouble offset);
  bool selectStroke(const GLint size, const GLuint* buffer);
  void setPlanes(const GLint size_first, const GLfloat* buffer_first,
		 const GLint size_last, const GLfloat* buffer_last);
  bool addPoint2D(const GLint x, const GLint y);
  void addPoint(const GLint x, const GLint y);
  GLdouble getFirstPlane() const;
  GLdouble getLastPlane() const;
  GLuint selectedStrokeID() const;
  int projectionMode() const;
  bool read(const char* name);
  bool write(const char* name) const;
  void fair();
  void draw() const;
  void draw2D() const;
  void clear();
  
  int window;
  GLint viewport[4];
  GLdouble fovy, aspect, near, far;
  GLdouble mv_matrix[16], proj_matrix[16];
  vec3 view_vector;
  
  std::vector<GLfloat> object_tokens;
  
  GLfloat point_size;
  
  /* Input from mouse or tablet */
  std::vector<point> positions;
  // Also pressure, tilt angle, etc. in the future!
  
  static const GLuint DEFAULT_NAME;
};

#endif // INPUT_H
