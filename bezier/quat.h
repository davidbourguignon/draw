#ifndef QUAT_H
#define QUAT_H

#include "vec3.h"



// -------------------------------------------------
//  
//  Quat class.
//  Unit quaternion used to model a rotation in 3D.
//  
//  François Faure.
//  
// -------------------------------------------------

template <class Real = double>

class Quat
{

public:
  
  /* Constructors/Destructor */
  
  // Default constructor: does nothing
  Quat();
  
  // Components w, x, y, z
  Quat(const Real w, const Real x, const Real y, const Real z);
  
  // Axis and angle of rotation
  Quat(const Vec3<Real>& axis, const Real angle);
  
  
  /* Get */
  
  // Get axis, angle -biased!- for display only
  void getAxisAngle(Real& x, Real& y, Real& z, Real& angle) const;
  
  // Get axis, angle -biased!- for display only
  void getAxisAngle(Vec3<Real>& axis, Real& angle) const ;
  
  // Get w value
  const Real& w() const;
  
  // Get x value
  const Real& x() const;
  
  // Get y value
  const Real& y() const;
  
  // Get z value
  const Real& z() const;
  
  
  /* Set */
  
  /* Set axis, angle.
     Axis represented by 3 Real (axis needs not to be normalized). */
  void setAxisAngle(const Real x, const Real y, const Real z,
		    const Real angle);
  
  /* Set axis, angle.
     Axis is represented by a vector (axis needs not to be normalized). */
  void setAxisAngle(const Vec3<Real>& axis, const Real angle);
  
  // Set w value
  Quat& setw(const Real );
  
  // Set x value
  Quat& setx(const Real );
  
  // Set y value
  Quat& sety(const Real );
  
  // Set z value
  Quat& setz(const Real );
  
  
  /* Operations on Quats (rotations composition) */
  
  // In-place product
  Quat& operator*=(const Quat& );
  
  // In-place product with the inverse of a Quat
  Quat& operator/=(const Quat& );
  
  // Product with a Quat
  Quat operator*(const Quat& ) const;
  
  // Product with the inverse of a Quat
  Quat operator/(const Quat& ) const ;
  
  // Inverse rotation
  Quat inv() const ;
  
  
  /* Operations on vectors */
  
  // Apply rotation to a vector
  Vec3<Real> operator*(const Vec3<Real>& ) const;
  
  // Apply rotation to a vector
  template <class R>
  friend Vec3<R>
  operator*(const Vec3<R>& , const Quat<R>& );
  
  // Apply inverse rotation to a vector
  Vec3<Real> operator/(const Vec3<Real>& ) const;
  
  // Apply inverse rotation to a vector
  template <class R>
  friend Vec3<R>
  operator/(const Vec3<R>& , const Quat<R>& );
  
  
  /* Input/Output */
  
  // Text output: axis angle
  template <class R>
  friend std::ostream&
  operator<<(std::ostream& , const Quat<R>& );
  
  // Text input: axis angle
  template <class R>
  friend std::istream&
  operator>>(std::istream& , Quat<R>& );
  
  
  /* Norm */
  
  // Norm
  Real norm() const;
  
  // Normalize
  Quat& normalize();
  
  
  /* Constants */
  
  // Identity (rotation with null angle)
  static const Quat id();
  
  
private:
  
  Real v[4]; // The Quat is modeled using (w, x, y, z)
             // i.e. cos(t/2), ax.sin(t/2), ay.sin(t/2), az.sin(t/2)
             // where {ax,ay,az} is a quaternion axis with unit norm
             // and t is the quaternion angle
  
}; // class Quat



// -----------------------------
//  
//  Quat class-related methods.
//  
//  François Faure.
//  
// -----------------------------

/* Quat to rotation matrix */

// Write rotation matrix m corresponding to Quat q
template <class Real>
void writeRotMatrix(const Quat<Real>& q, Real m[3][3]);

/* Write rotation matrix m corresponding to Quat q, in OpenGL format.
    For normalized Quat only! */

// Standard OpenGL matrix format
template <class Real>
void writeOpenGLRotMatrix(const Quat<Real>& q, Real m[16]);

// Free style
template <class Real>
void writeOpenGLRotMatrix(const Quat<Real>& q, Real m[4][4]);










// -------------------------------------------------------------
// 
//   D E F I N I T I O N   O F   I N L I N E D   M E T H O D S 
// 
// -------------------------------------------------------------

template<class Real>
inline
Quat<Real>::
Quat()
{}

template<class Real>
inline
Quat<Real>::
Quat(const Real w, const Real x, const Real y, const Real z)
{
  v[0]=w; v[1]=x; v[2]=y; v[3]=z;
}

template<class Real>
inline
Quat<Real>::
Quat(const Vec3<Real>& axis, const Real angle)
{
  setAxisAngle(axis, angle); 
}





template<class Real>
inline void 
Quat<Real>::
getAxisAngle(Real& x, Real& y, Real& z, Real& angle) const 
{
  angle = 2.0*Numerics<Real>::acosine( v[0]>1.0 ? 1.0 : v[0]<-1.0 ? -1.0 : v[0] );
  
  if ( angle > Numerics<Real>::numthreshold() )
    {
      Real tmp = 1.0/Numerics<Real>::sine(angle/2);
      x=v[1]*tmp;
      y=v[2]*tmp;
      z=v[3]*tmp;
    }
  else
    {
      x = 1.0; // Arbitrary!
      y = 0.0;
      z = 0.0;
    }
}

template<class Real>
inline void 
Quat<Real>::
getAxisAngle(Vec3<Real>& axis, Real& angle) const 
{
  angle = 2.0*Numerics<Real>::acosine( v[0]>1.0 ? 1.0 : v[0]<-1.0 ? -1.0 : v[0] );
  
  if ( angle > Numerics<Real>::numthreshold() )
    {
      Real tmp = 1.0/Numerics<Real>::sine(angle/2);
      axis = Vec3<Real>(v[1]*tmp, v[2]*tmp, v[3]*tmp);
    }
  else
    {
      axis = Vec3<Real>(1.0, 0.0, 0.0); // Arbitrary!
    }
}

template<class Real>
inline const Real&
Quat<Real>::
w() const
{
  return v[0];
}

template<class Real>
inline const Real&
Quat<Real>::
x() const
{
  return v[1];
}

template<class Real>
inline const Real&
Quat<Real>::
y() const
{
  return v[2];
}

template<class Real>
inline const Real&
Quat<Real>::
z() const
{
  return v[3];
}





template<class Real>
inline void 
Quat<Real>::
setAxisAngle(const Real x, const Real y, const Real z, const Real angle)
{
  v[0] = Numerics<Real>::cosine(angle/2);
  
  Vec3<Real> axisn(x, y, z);
  axisn.normalize();
  Real tmp = Numerics<Real>::sine(angle/2);
  
  v[1] = axisn.x()*tmp;
  v[2] = axisn.y()*tmp;
  v[3] = axisn.z()*tmp;
}

template<class Real>
inline void 
Quat<Real>::
setAxisAngle(const Vec3<Real>& axis, const Real angle)
{
  v[0] = Numerics<Real>::cosine(angle/2);
  
  Vec3<Real> axisn = axis;
  axisn.normalize();
  Real tmp = Numerics<Real>::sine(angle/2);
  
  v[1] = axisn.x()*tmp;
  v[2] = axisn.y()*tmp;
  v[3] = axisn.z()*tmp;
}

template <class Real>
inline Quat<Real>&
Quat<Real>::
setw(const Real w)
{
  v[0]=w;
  return *this;
}

template <class Real>
inline Quat<Real>&
Quat<Real>::
setx(const Real x)
{
  v[1]=x;
  return *this;
}

template <class Real>
inline Quat<Real>&
Quat<Real>::
sety(const Real y)
{
  v[2]=y;
  return *this;
}

template <class Real>
inline Quat<Real>&
Quat<Real>::
setz(const Real z)
{
  v[3]=z;
  return *this;
}





template<class Real>
inline Quat<Real>& 
Quat<Real>::
operator*=(const Quat& q)
{
  Real w = q.v[0]*v[0] - q.v[1]*v[1] - q.v[2]*v[2] - q.v[3]*v[3];
  Real x = q.v[0]*v[1] + q.v[1]*v[0] + q.v[2]*v[3] - q.v[3]*v[2];
  Real y = q.v[0]*v[2] - q.v[1]*v[3] + q.v[2]*v[0] + q.v[3]*v[1];
  Real z = q.v[0]*v[3] + q.v[1]*v[2] - q.v[2]*v[1] + q.v[3]*v[0];
  v[0] = w; v[1] = x; v[2] = y; v[3] = z;
  
  return *this;
}

template<class Real>
inline Quat<Real>& 
Quat<Real>::
operator/=(const Quat& q)
{
  Real w = q.v[0]*v[0] + q.v[1]*v[1] + q.v[2]*v[2] + q.v[3]*v[3];
  Real x = q.v[0]*v[1] - q.v[1]*v[0] - q.v[2]*v[3] + q.v[3]*v[2];
  Real y = q.v[0]*v[2] + q.v[1]*v[3] - q.v[2]*v[0] - q.v[3]*v[1];
  Real z = q.v[0]*v[3] - q.v[1]*v[2] + q.v[2]*v[1] - q.v[3]*v[0];
  v[0] = w; v[1] = x; v[2] = y; v[3] = z;
  
  return *this;
}

template<class Real>
inline Quat<Real>
Quat<Real>::
operator*(const Quat& q) const
{
  return Quat
   (
     q.v[0]*v[0] - q.v[1]*v[1] - q.v[2]*v[2] - q.v[3]*v[3],
     q.v[0]*v[1] + q.v[1]*v[0] + q.v[2]*v[3] - q.v[3]*v[2],
     q.v[0]*v[2] - q.v[1]*v[3] + q.v[2]*v[0] + q.v[3]*v[1],
     q.v[0]*v[3] + q.v[1]*v[2] - q.v[2]*v[1] + q.v[3]*v[0]
   );	
}

template<class Real>
inline Quat<Real>
Quat<Real>::
operator/(const Quat& q) const
{
  return Quat
   (
     q.v[0]*v[0] + q.v[1]*v[1] + q.v[2]*v[2] + q.v[3]*v[3],
     q.v[0]*v[1] - q.v[1]*v[0] - q.v[2]*v[3] + q.v[3]*v[2],
     q.v[0]*v[2] + q.v[1]*v[3] - q.v[2]*v[0] - q.v[3]*v[1],
     q.v[0]*v[3] - q.v[1]*v[2] + q.v[2]*v[1] - q.v[3]*v[0]
   );	
}

template<class Real>
inline Quat<Real>
Quat<Real>::
inv() const
{
  return Quat(v[0], -v[1], -v[2], -v[3]);
}





template <class Real>
inline Vec3<Real>
Quat<Real>::
operator*(const Vec3<Real>& p) const 
{
  Real r[4];
  r[0] =  p.x()*v[1] + p.y()*v[2] + p.z()*v[3];
  r[1] =  p.x()*v[0] - p.y()*v[3] + p.z()*v[2];
  r[2] =  p.x()*v[3] + p.y()*v[0] - p.z()*v[1];
  r[3] = -p.x()*v[2] + p.y()*v[1] + p.z()*v[0];
  
  return Vec3<Real>
   (
     v[0]*r[1] + v[1]*r[0] + v[2]*r[3] - v[3]*r[2],
     v[0]*r[2] - v[1]*r[3] + v[2]*r[0] + v[3]*r[1],
     v[0]*r[3] + v[1]*r[2] - v[2]*r[1] + v[3]*r[0]
   );

}

template <class Real>
inline Vec3<Real>
operator*(const Vec3<Real>& p, const Quat<Real>& q)
{
  return q*p;
}

template <class Real>
inline Vec3<Real>
Quat<Real>::
operator/(const Vec3<Real>& p) const
{
  Real r[4];
  r[0] = -p.x()*v[1] - p.y()*v[2] - p.z()*v[3];
  r[1] =  p.x()*v[0] + p.y()*v[3] - p.z()*v[2];
  r[2] = -p.x()*v[3] + p.y()*v[0] + p.z()*v[1];
  r[3] =  p.x()*v[2] - p.y()*v[1] + p.z()*v[0];
  
  return Vec3<Real>
   (
     v[0]*r[1] - v[1]*r[0] - v[2]*r[3] + v[3]*r[2],
     v[0]*r[2] + v[1]*r[3] - v[2]*r[0] - v[3]*r[1],
     v[0]*r[3] - v[1]*r[2] + v[2]*r[1] - v[3]*r[0]
   );
}

template <class Real>
inline Vec3<Real>
operator/(const Vec3<Real>& p, const Quat<Real>& q)
{
  return q/p;
}





template <class Real>
inline std::ostream& 
operator<<(std::ostream& stream, const Quat<Real>& q)
{
  Real x, y, z, angle;
  q.getAxisAngle(x,y,z,angle);
  stream << x << " " << y << " " << z << " " << angle;
  
  return stream;
}

template <class Real>
inline std::istream& 
operator>>(std::istream& stream, Quat<Real>& q)
{
  Real x, y, z, angle;
  stream >> x >> y >> z >> angle;
  q.setAxisAngle(x,y,z,angle);
  
  return stream;
}





template<class Real>
inline Real
Quat<Real>::
norm() const
{
  return Numerics<Real>::sqroot( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3] );
}

template<class Real>
inline Quat<Real>&
Quat<Real>::
normalize()
{
  Real tmp = 1.0/norm();
  v[0]*=tmp; v[1]*=tmp; v[2]*=tmp; v[3]*=tmp;
  return *this;
}





template<class Real>
inline const Quat<Real>
Quat<Real>::
id()
{
  return Quat(Real(1.0), Real(0.0), Real(0.0), Real(0.0));
}





template <class Real>
inline void
writeRotMatrix(const Quat<Real>& q, Real m[3][3])
{
  Real qw = q.w();
  Real qx = q.x();
  Real qy = q.y();
  Real qz = q.z();
  
  Real qww = qw*qw;
  Real qwx = qw*qx;
  Real qwy = qw*qy;
  Real qwz = qw*qz;
  
  Real qxx = qx*qx;
  Real qxy = qx*qy;
  Real qxz = qx*qz;
  
  Real qyy = qy*qy;
  Real qyz = qy*qz;
  
  Real qzz = qz*qz;
  
  /* Standard C convention:
     the element m[i][j] is in row i and column j.
  */
  m[0][0] = qww + qxx - (qyy + qzz);
  m[1][0] =         2.0*(qxy - qwz);
  m[2][0] =         2.0*(qxz + qwy);
  
  m[0][1] =         2.0*(qxy + qwz);
  m[1][1] = qww + qyy - (qzz + qxx);
  m[2][1] =         2.0*(qyz - qwx);
  
  m[0][2] =         2.0*(qxz - qwy);
  m[1][2] =         2.0*(qyz + qwx);
  m[2][2] = qww + qzz - (qyy + qxx);
}

template <class Real>
inline void
writeOpenGLRotMatrix(const Quat<Real>& q, Real m[16])
{
  Real qw = q.w();
  Real qx = q.x();
  Real qy = q.y();
  Real qz = q.z();
  
  Real qwx = qw*qx;
  Real qwy = qw*qy;
  Real qwz = qw*qz;
  
  Real qxx = qx*qx;
  Real qxy = qx*qy;
  Real qxz = qx*qz;
  
  Real qyy = qy*qy;
  Real qyz = qy*qz;
  
  Real qzz = qz*qz;
  
  /* OpenGL convention:
     the element m[i][j] is in the ith column and jth row
     of the OpenGL transformation matrix.
     This is the reverse of the standard C convention.
  */
  m[0]  = 1.0 - 2.0*(qyy + qzz);
  m[1]  =       2.0*(qxy - qwz);
  m[2]  =       2.0*(qxz + qwy);
  m[3]  = 0.0;
  
  m[4]  =       2.0*(qxy + qwz);
  m[5]  = 1.0 - 2.0*(qzz + qxx);
  m[6]  =       2.0*(qyz - qwx);
  m[7]  = 0.0;
  
  m[8]  =       2.0*(qxz - qwy);
  m[9]  =       2.0*(qyz + qwx);
  m[10] = 1.0 - 2.0*(qyy + qxx);
  m[11] = 0.0;
  
  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;
}

template <class Real>
inline void
writeOpenGLRotMatrix(const Quat<Real>& q, Real m[4][4])
{
  Real qw = q.w();
  Real qx = q.x();
  Real qy = q.y();
  Real qz = q.z();
  
  Real qwx = qw*qx;
  Real qwy = qw*qy;
  Real qwz = qw*qz;
  
  Real qxx = qx*qx;
  Real qxy = qx*qy;
  Real qxz = qx*qz;
  
  Real qyy = qy*qy;
  Real qyz = qy*qz;
  
  Real qzz = qz*qz;
  
  /* OpenGL convention:
     the element m[i][j] is in the ith column and jth row
     of the OpenGL transformation matrix.
     This is the reverse of the standard C convention.
  */
  m[0][0] = 1.0 - 2.0*(qyy + qzz);
  m[0][1] =       2.0*(qxy - qwz);
  m[0][2] =       2.0*(qxz + qwy);
  m[0][3] = 0.0;
  
  m[1][0] =       2.0*(qxy + qwz);
  m[1][1] = 1.0 - 2.0*(qzz + qxx);
  m[1][2] =       2.0*(qyz - qwx);
  m[1][3] = 0.0;
  
  m[2][0] =       2.0*(qxz - qwy);
  m[2][1] =       2.0*(qyz + qwx);
  m[2][2] = 1.0 - 2.0*(qyy + qxx);
  m[2][3] = 0.0;
  
  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 1.0;
}



#endif // QUAT_H
