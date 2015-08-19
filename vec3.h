#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include "numerics.h"



// --------------------------------
//  
//  Vec3 class.
//  Vector (or point) in 3D space.
//  
//  David Bourguignon.
//  
// --------------------------------

template <class Real = double>

class Vec3
{

public:
  
  /* Constructors/Destructor */
  
  // Default constructor: does nothing
  Vec3();
  
  // Constructor: create a Vec3 with 3 Real values
  Vec3(const Real , const Real , const Real );
  
  
  /* Get */
  
  // Get x value
  const Real& x() const;
  
  // Get y value
  const Real& y() const;
  
  // Get z value
  const Real& z() const;
  
  // Operator Vec3[i]
  const Real& operator[](int i) const;
  
  
  /* Set */
  
  // Set x value
  Vec3& setx(const Real );
  
  // Set y value
  Vec3& sety(const Real );
  
  // Set z value
  Vec3& setz(const Real );
  
  // Operator Vec3[i]
  Real& operator[](int i);
  
  
  /* Assignment operators */
  
  // Operator Vec3 += Vec3
  Vec3& operator+=(const Vec3& );
  
  // Operator Vec3 -= Vec3
  Vec3& operator-=(const Vec3& );
  
  // Operator Vec3 *= Real
  Vec3& operator*=(const Real );
  
  // Operator Vec3 /= Real
  Vec3& operator/=(const Real );
  
  
  /* Arithmetic operators */
  
  // Operator Vec3+Vec3
  Vec3 operator+(const Vec3& ) const;
  
  // Operator Vec3-Vec3
  Vec3 operator-(const Vec3& ) const;
  
  // Operator Vec3*Real
  Vec3 operator*(const Real ) const;
  
  // Operator Real*Vec3
  template <class R>
  friend Vec3<R>
  operator*(const R , const Vec3<R>& );
  
  // Operator Vec3/Real
  Vec3 operator/(const Real ) const;
  
  // Operator -Vec3
  Vec3 operator-();
  
  
  /* Equality and relational operators */
  
  // Operator Vec3 == Vec3
  bool operator==(const Vec3& ) const;
  
  // Operator Vec3 != Vec3
  bool operator!=(const Vec3& ) const;
  
  // Operator Vec3 < Vec3
  bool operator<(const Vec3& ) const;
  
  
  /* I/O stream operators */
  
  // Operator <<
  template <class R>
  friend std::ostream&
  operator<<(std::ostream& , const Vec3<R>& );
  
  // Operator >>
  template <class R>
  friend std::istream&
  operator>>(std::istream& , Vec3<R>& );
  
  
  /* Norms */
  
  // Norm (aka l-2 norm, euclidean norm)
  Real norm() const;
  
  // Square norm
  Real sqnorm() const;
  
  // Infinite norm (aka l-infinity norm)
  Real inftNorm() const;
  
  // Normalization
  Vec3& normalize();
  
  
  /* Constants */
  
  // Null Vec3
  static const Vec3 null();
  
  // Size
  static int size();
  
  
private:
  
  Real a[3];
  
}; // class Vec3



// -----------------------------
//  
//  Vec3 class-related methods.
//  
//  David Bourguignon.
//  
// -----------------------------

/* Dot and cross products */

// Dot product
template <class Real>
Real dot(const Vec3<Real>& , const Vec3<Real>& );

// Cross product
template <class Real>
Vec3<Real> cross(const Vec3<Real>& , const Vec3<Real>& );


// Distance between p1 and p2
template <class Real>
Real dist(const Vec3<Real>& p1, const Vec3<Real>& p2);

// Square distance between p1 and p2
template <class Real>
Real sqdist(const Vec3<Real>& p1, const Vec3<Real>& p2);

// Cosinus of the (v1,v2) angle
template <class Real>
Real cosAng(const Vec3<Real>& v1, const Vec3<Real>& v2);










// -------------------------------------------------------------
// 
//   D E F I N I T I O N   O F   I N L I N E D   M E T H O D S 
// 
// -------------------------------------------------------------

template <class Real>
inline
Vec3<Real>::
Vec3()
{}

template <class Real>
inline
Vec3<Real>::
Vec3(const Real x0, const Real y0, const Real z0)
{
  a[0] = x0; a[1] = y0; a[2] = z0;
}





template <class Real>
inline const Real&
Vec3<Real>::
x() const 
{
  return a[0];
}

template <class Real>
inline const Real&
Vec3<Real>::
y() const 
{
  return a[1];
}

template <class Real>
inline const Real&
Vec3<Real>::
z() const 
{
  return a[2];
}

template <class Real>
inline const Real&
Vec3<Real>::
operator[](int i) const
{
  return a[i];
}





template <class Real>
inline Vec3<Real>&
Vec3<Real>::
setx(const Real x0)
{
  a[0] = x0;
  return *this;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
sety(const Real y0)
{
  a[1] = y0;
  return *this;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
setz(const Real z0)
{
  a[2] = z0;
  return *this;
}

template <class Real>
inline Real&
Vec3<Real>::
operator[](int i)
{
  return a[i];
}





template <class Real>
inline Vec3<Real>&
Vec3<Real>::
operator+=(const Vec3& v)
{
  a[0] += v.x();
  a[1] += v.y();
  a[2] += v.z();
  return *this;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
operator-=(const Vec3& v)
{
  a[0] -= v.x();
  a[1] -= v.y();
  a[2] -= v.z();
  return *this;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
operator*=(const Real k)
{
  a[0] *= k;
  a[1] *= k;
  a[2] *= k;
  return *this;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
operator/=(const Real k)
{
  (*this) *= 1.0/k;
  return *this;
}





template <class Real>
inline Vec3<Real>
Vec3<Real>::
operator+(const Vec3& v) const
{
  return Vec3( a[0]+v.x(), a[1]+v.y(), a[2]+v.z() );
}

template <class Real>
inline Vec3<Real>
Vec3<Real>::
operator-(const Vec3& v) const
{
  return Vec3( a[0]-v.x(), a[1]-v.y(), a[2]-v.z() );
}

template <class Real>
inline Vec3<Real>
Vec3<Real>::
operator*(const Real k) const
{
  return Vec3(a[0]*k, a[1]*k, a[2]*k);
}

template <class Real>
inline Vec3<Real>
operator*(const Real k, const Vec3<Real>& v)
{
  return v*k;
}

template <class Real>
inline Vec3<Real>
Vec3<Real>::
operator/(const Real k) const
{
  return (*this)*(1.0/k);
}

template <class Real>
inline Vec3<Real>
Vec3<Real>::
operator-()
{
  return Vec3( -x(), -y(), -z() );
}





template <class Real>
inline bool
Vec3<Real>::
operator==(const Vec3& v) const
{
  return ( (v.x() == a[0]) && (v.y() == a[1]) && (v.z() == a[2]) );
}

template <class Real>
inline bool
Vec3<Real>::
operator!=(const Vec3& v) const
{
  return !operator==(v);
}

template <class Real>
inline bool
Vec3<Real>::
operator<(const Vec3& v) const
{
  return ( (a[0] < v.x()) && (a[1] < v.y()) && (a[2] < v.z()) );
}





template <class Real>
inline std::ostream&
operator<<(std::ostream& s, const Vec3<Real>& v)
{
  return s << v.x() << " " << v.y() << " " << v.z();
}

template <class Real>
inline std::istream&
operator>>(std::istream& s, Vec3<Real>& v)
{
  Real x0 = 0.0;
  Real y0 = 0.0;
  Real z0 = 0.0;
  
  s >> x0 >> y0 >> z0;
  v = Vec3<Real>(x0, y0, z0);
  return s;
}





template <class Real>
inline Real
Vec3<Real>::
norm() const 
{
  return Numerics<Real>::sqroot( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
}

template <class Real>
inline Real
Vec3<Real>::
sqnorm() const
{
  return ( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
}

template <class Real>
inline Real
Vec3<Real>::
inftNorm() const
{
  Real x0 = a[0];
  Real y0 = a[1];
  Real z0 = a[2];
  
  if ( x0 < 0.0 ) x0 = -x0;
  if ( y0 < 0.0 ) y0 = -y0;
  if ( z0 < 0.0 ) z0 = -z0;
  
  Real max = x0;
  
  if ( max < y0 ) max = y0;
  if ( max < z0 ) max = z0;
  return max;
}

template <class Real>
inline Vec3<Real>&
Vec3<Real>::
normalize()
{
  (*this) *= 1.0/norm();
  return *this;
}





template <class Real>
inline const Vec3<Real>
Vec3<Real>::
null()
{
  return Vec3( Real(0.0), Real(0.0), Real(0.0) );
}

template <class Real>
inline int
Vec3<Real>::
size()
{
  return 3;
}





template <class Real>
inline Real
dot(const Vec3<Real>& v1, const Vec3<Real>& v2)
{
  return ( v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z() );
}

template <class Real>
inline Vec3<Real>
cross(const Vec3<Real>& v1, const Vec3<Real>& v2)
{
  return Vec3<Real>( v1.y()*v2.z() - v1.z()*v2.y(),
		     v1.z()*v2.x() - v1.x()*v2.z(),
		     v1.x()*v2.y() - v1.y()*v2.x() );
}





template <class Real>
inline Real
dist(const Vec3<Real>& p1, const Vec3<Real>& p2)
{
  return (p1 - p2).norm();
}

template <class Real>
inline Real
sqdist(const Vec3<Real>& p1, const Vec3<Real>& p2)
{
  return (p1 - p2).sqnorm();
}

template <class Real>
inline Real
cosAng(const Vec3<Real>& v1, const Vec3<Real>& v2)
{
  return dot(v1,v2)/Numerics<Real>::sqroot( v1.sqnorm()*v2.sqnorm() );
}



#endif // VEC3_H
