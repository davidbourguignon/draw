#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "quat.h"



// ------------------------------------------------------------------
//  
//  Trackball class.
//  Trackball in 3D space.
//  
//  Rewritten from trackball class by Pierre-Olivier Agliati (1999).
//  See SIGGRAPH '96 Advanced OpenGL course notes.
//  
//  David Bourguignon.
//  
// ------------------------------------------------------------------

template <class Real = double>

class Trackball
{

public:
  
  /* Constructor: create a Trackball, specifying
     initial rotation and translation values,
     translation sensitivity and size */
  Trackball(Quat<Real> r_i = Quat<Real>::id(),
	    Vec3<Real> t_i = Vec3<Real>::null(),
	    Real t_s = 10.0, Real s = 1.0);
  
  
  /* Get */
  
  // Get current state
  bool isMoved() const;
  
  
  /* Set */
  
  // Set to a given transformation
  void setTransf(const Quat<Real>& , const Vec3<Real>& );
  
  // Set initial transformation
  void setInitTransf(const Quat<Real>& , const Vec3<Real>& );
  
  // Reset to initial transformation
  void reinitializeTransf();
  
  // Set translation sensitivity
  void setTranslSenst(Real );
  
  // Set size
  void setSize(Real );
  
  /* Set active range of the mouse (width, height)
     Call it when trackball is created or window viewport changes */
  void reshape(int w, int h);
  
  // Set state
  void change(const bool choice = true);
  
  
  /* Motion operations */
  
  // Write current transformation matrix, in OpenGL format
  void writeOpenGLTransfMatrix(Real m[16]) const;
  void writeOpenGLTransfMatrix(Real m[4][4]) const;
  
  // Start movement
  void startRotation(int x, int y);
  void startTranslationXY(int x, int y);
  void startTranslationZ(int x, int y);
  
  // Movement
  void move(int x, int y);
  
  // Stop movement
  void stopRotation();
  void stopTranslationXY();
  void stopTranslationZ();
  
  
private:
  
  // Projection to deformed ball
  Real projectionToDeformedBall(Real x, Real y) const;
  
  // Build new quaternion when rotation is performed
  void buildQuaternion(Quat<Real> &q,
		       Real p1x, Real p1y,
		       Real p2x, Real p2y) const;
  
  // Compute transformation according to motion performed
  void moveRotation(int x, int y);
  void moveTranslationXY(int x, int y);
  void moveTranslationZ(int x, int y);
  
  
  // A trackball is defined using a quaternion and
  // a translation vector
  Quat<Real> rot, rot_init;
  Vec3<Real> transl, transl_init;
  
  // Translation sensitivity, size
  Real transl_senst, size;
  
  // Active range for x & y coordinates
  int width, height;
  
  // Last defined coordinates
  int last_x, last_y;
  
  // Current state
  bool is_rotd, is_transldXY, is_transldZ, is_changed;
  
  // Square size, size limit (to handle size problems)
  Real sqsize, size_limit;
  
}; // class Trackball










// -------------------------------------------------------------
// 
//   D E F I N I T I O N   O F   I N L I N E D   M E T H O D S 
// 
// -------------------------------------------------------------

template <class Real>
inline
Trackball<Real>::
Trackball(Quat<Real> r_i, Vec3<Real> t_i, Real t_s, Real s)
: rot(r_i), rot_init(r_i),
  transl(t_i), transl_init(t_i),
  transl_senst(t_s),
  size(s), sqsize(size*size), size_limit(sqsize*0.5),
  width(), height(),
  last_x(), last_y(),
  is_rotd(false), is_transldXY(false), is_transldZ(false), is_changed(false)
{}





template <class Real>
inline bool Trackball<Real>::
isMoved() const {
  return (is_rotd || is_transldXY || is_transldZ || is_changed);
}





template <class Real>
inline void
Trackball<Real>::
setTransf(const Quat<Real>& q, const Vec3<Real>& t)
{
  is_changed = true;
  
  rot = q;
  transl = t;
}

template <class Real>
inline void
Trackball<Real>::
setInitTransf(const Quat<Real>& q, const Vec3<Real>& t)
{
  rot_init = q;
  transl_init = t;
}

template <class Real>
inline void
Trackball<Real>::
reinitializeTransf()
{
  setTransf(rot_init, transl_init);
}

template <class Real>
inline void
Trackball<Real>::
setTranslSenst(Real t_s)
{
  transl_senst = t_s;
}

template <class Real>
inline void
Trackball<Real>::
setSize(Real s)
{
  size = s;
  sqsize = size*size;
  size_limit = sqsize*0.5;
}

template <class Real>
inline void
Trackball<Real>::
reshape(int w, int h)
{
  width = w; height = h;
}

template <class Real>
inline void
Trackball<Real>::
change(const bool choice)
{
  is_changed = choice;
}





template <class Real>
inline Real
Trackball<Real>::
projectionToDeformedBall(Real x, Real y) const
{
  Real d = x*x + y*y;
  
  // Return z coordinate (?), i.e.
  // distance to ball center for small distances, with sqsize = d + z*z
  // else a decreasing value to zero
  if ( d < size_limit )
    return Numerics<Real>::sqroot(sqsize - d);
  else
    return ( size_limit / Numerics<Real>::sqroot(d) );
}

template <class Real>
inline void
Trackball<Real>::
buildQuaternion(Quat<Real>& q,
		Real p1x, Real p1y, Real p2x, Real p2y) const
{
  // Case of null rotation
  if ( (p1x == p2x) && (p1y == p2y) )
    {
      q = Quat<Real>::id();
      return;
    }
  
  // Points coordinates are reported in the new 3D frame:
  // origin at ball center,
  // x (respectively y) unit vector length equal to half screen width
  // (respectively height),
  // z unit vector length approximately equal to distance from screen
  // plane to ball center (?).
  Vec3<Real> p1( p1x, p1y, projectionToDeformedBall(p1x,p1y) );
  Vec3<Real> p2( p2x, p2y, projectionToDeformedBall(p2x,p2y) );
  
  // Axis of rotation
  Vec3<Real> axis = cross(p2,p1);
  axis.normalize();
  
  // Approximation of rotation half-angle sinus
  Real sin_ha = (p1-p2).norm()/(2.0*size);
  
  // Clamp "out-of-control" values
  if ( sin_ha > 1.0 ) sin_ha = 1.0;
  else if ( sin_ha < -1.0 ) sin_ha = -1.0;
  
  Real cos_ha = Numerics<Real>::sqroot(1.0 - sin_ha*sin_ha);
  
  // Update quaternion
  q.setw( cos_ha          );
  q.setx( axis.x()*sin_ha );
  q.sety( axis.y()*sin_ha );
  q.setz( axis.z()*sin_ha );
}

template <class Real>
inline void
Trackball<Real>::
moveRotation(int x, int y)
{
  // Compute transformation quaternion
  Quat<Real> q;
  buildQuaternion( q,
    static_cast<Real>( (2*last_x - width)  ) / static_cast<Real>(width ),
    static_cast<Real>( (height - 2*last_y) ) / static_cast<Real>(height),
    static_cast<Real>( (2*x - width)       ) / static_cast<Real>(width ),
    static_cast<Real>( (height - 2*y)      ) / static_cast<Real>(height) );
  // Points coordinates are reported in the new 2D frame:
  // origin at screen center,
  // x (respectively y) unit vector length equal to half screen width
  // (respectively height).
  
  // Add to the global quaternion
  rot = q*rot;
}

template <class Real>
inline void
Trackball<Real>::
moveTranslationXY(int x, int y)
{
  transl[0] += (x - last_x)/transl_senst;
  transl[1] += (last_y - y)/transl_senst;
}

template <class Real>
inline void
Trackball<Real>::
moveTranslationZ(int x, int y)
{
  transl[2] += (y - last_y)/transl_senst;
}





template <class Real>
inline void
Trackball<Real>::
writeOpenGLTransfMatrix(Real m[16]) const
{
  /* We fill the transformation matrix */
  
  // Rotation & Scaling
  writeOpenGLRotMatrix(rot,m);
  
  // Translation
  m[12] = transl[0];
  m[13] = transl[1];
  m[14] = transl[2];
}

template <class Real>
inline void
Trackball<Real>::
writeOpenGLTransfMatrix(Real m[4][4]) const
{
  /* We fill the transformation matrix */
  
  // Rotation & Scaling
  writeOpenGLRotMatrix(rot,m);
  
  // Translation
  m[3][0] = transl[0];
  m[3][1] = transl[1];
  m[3][2] = transl[2];
}

template <class Real>
inline void
Trackball<Real>::
startRotation(int x, int y)
{
  is_rotd = true;
  is_changed = false;
  
  // Save current parameters
  last_x = x; last_y = y;
}

template <class Real>
inline void
Trackball<Real>::
startTranslationXY(int x, int y)
{
  is_transldXY = true;
  is_changed = false;
  
  // Save current parameters
  last_x = x; last_y = y;
}

template <class Real>
inline void
Trackball<Real>::
startTranslationZ(int x, int y)
{
  is_transldZ = true;
  is_changed = false;
  
  // Save current parameters
  last_x = x; last_y = y;
}

template <class Real>
inline void
Trackball<Real>::
move(int x, int y)
{
  // Handle every enabled transformation
  if ( is_rotd )
    moveRotation(x,y);
  
  if ( is_transldXY )
    moveTranslationXY(x,y);
  
  if ( is_transldZ )
    moveTranslationZ(x,y);
  
  // Save current parameters
  last_x = x; last_y = y;
}

template <class Real>
inline void
Trackball<Real>::
stopRotation()
{
  is_rotd = false;
}

template <class Real>
inline void
Trackball<Real>::
stopTranslationXY()
{
  is_transldXY = false;
}

template <class Real>
inline void
Trackball<Real>::
stopTranslationZ()
{
  is_transldZ = false;
}



#endif // TRACKBALL_H
