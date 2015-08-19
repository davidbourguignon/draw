#ifndef STROKE3D_H
#define STROKE3D_H

#include <list>
#include <bezier_surface.h>
#include <aabb.h>
#include "opengl_utils.h"
#include "stroke2D.h"

class Stroke3D {
private:
  typedef GLdouble                      real;
  typedef Vec3<real>                    vec3;
  typedef Bezier<real, vec3>            bezier_simple;
  typedef Bezier_Augmented<real, vec3>  bezier;
  typedef Bezier_Surface<real, vec3>    bezier_surface;
  typedef AABB<real, vec3>              bounding_box;
  typedef std::vector<bezier>           beziers;
  typedef std::vector<bezier_simple>    beziers_simples;
  typedef std::vector<bezier_surface>   beziers_surfaces;
  enum texturemode {NO_TEXTURE, TEXTURE_1D, TEXTURE_2D};
  
  void computeBezierSurface();
  void computeBoundingBox();
  void computeBarycenter();
  void probaSurface(/*const GLint nstep_u, */const GLint nstep_v,
		    const int texture_mode) const;
  void initSteps();
  void computeMeanRadius();
  void computeNormals();
  void buildDisplayLists(const int window);
  void initDisplayData();
  void setClippingPlanesEqns();
  
  beziers_surfaces proba_surface;
  
  // Move data
  vec3 view_vector_prev;
#if 0
  vec3 mean_normal; // Mean normal of Bezier curves
                    // (in the stroke plane)
  vec3 ortho_plane_normal; // Normal to orthogonal plane
                           // to stroke plane
#endif
  
  // References to intersected strokes
  std::list<Stroke3D*> pstrokes;
  
  // Clipping planes equations
  std::vector< std::vector<GLdouble> > equations;
  
  // Probability surface angle
  static const real psang;
  static const real cos_psang;
  static const real sin_psang;
  
  static const real steps_per_unit_length;
  
public:
  enum drawingmode {LINE, OCCLUSION, TEXTURED_POLYGON};
  
  Stroke3D();
  Stroke3D(const Input& in, const Stroke2D& s, const int mode = LINE);
  void setInitColor(const GLfloat c[4]);
  void setColor(const GLfloat c[4]);
  void reinitColor();
  void read(std::ifstream& file_in, const int window);
  void write(std::ofstream& file_out) const;
  bool empty() const;
  void move(const Input& in);
  void reverse(const int window);
  void translate(int first_x, int first_y, int last_x, int last_y,
		 const Input& in);
  
  void addIntersectedStroke(Stroke3D& p);
  void addPStroke(Stroke3D* p);
  void cleanIntersectedStrokes();
  void clean(Stroke3D* p);
  void clean(const int window);
  
  void drawSpline() const;
  void drawOccluder() const;
  void drawStroke() const;
  void drawStrokeFirstPass() const;
  void drawStrokeSecondPass() const;
  void drawProbaSurface() const;
  void drawProbaSurfacePicking() const;
  void drawIntersectedStrokes() const;
  void drawClippedStroke() const;
  
  void drawControlPoints() const;
  void drawTangents() const;
  void drawCurvatureVectors() const;
  void drawCircles() const;
  void drawNormals() const;
  void drawBarycenter() const;
  void drawBoundingBox() const;
  
  beziers bs;
  real length;       // Stroke length (in screen units)
  std::vector<real> relative_lengths;
  std::vector<GLint> nsteps;
  vec3 plane_normal; // Normal to stroke plane
  real mean_radius;
#if 0
  bool is_visible;
#endif
  bounding_box box;
  bezier::vec barycenter_global;
  
  GLuint proba_surface_list;
  GLuint proba_surface_picking_list;
  
  GLuint occluder_tex_name;
  GLuint proba_surface_tex_name;
  GLuint stroke_tex_name;
  
  int drawing_mode;
  GLfloat color_init[4];
  GLfloat color[4];
};

#endif // STROKE3D_H
