#ifndef VEC2_H
#define VEC2_H

#include <iostream>
#include "numerics.h"



// --------------------------------
//  
//  Vec2 class.
//  Vector (or point) in 2D space.
//  
//  David Bourguignon.
//  
// --------------------------------

template <class Real = double>

class Vec2
{

public:
  
  /* Constructors/Destructor */
  
  // Default constructor: does nothing
  Vec2();
  
  // Constructor: create a Vec2 with 2 Real values
  Vec2(const Real , const Real );
  
  
  /* Get */
  
  // Get x value
  const Real& x() const;
  
  // Get y value
  const Real& y() const;
  
  // Operator Vec2[i]
  const Real& operator[](int i) const;
  
  
  /* Set */
  
  // Set x value
  Vec2& setx(const Real );
  
  // Set y value
  Vec2& sety(const Real );
  
  // Operator Vec2[i]
  Real& operator[](int i);
  
  
  /* Assignment operators */
  
  // Operator Vec2 += Vec2
  Vec2& operator+=(const Vec2& );
  
  // Operator Vec2 -= Vec2
  Vec2& operator-=(const Vec2& );
  
  // Operator Vec2 *= Real
  Vec2& operator*=(const Real );
  
  // Operator Vec2 /= Real
  Vec2& operator/=(const Real );
  
  
  /* Arithmetic operators */
  
  // Operator Vec2+Vec2
  Vec2 operator+(const Vec2& ) const;
  
  // Operator Vec2-Vec2
  Vec2 operator-(const Vec2& ) const;
  
  // Operator Vec2*Real
  Vec2 operator*(const Real ) const;
  
  // Operator Real*Vec2
  template <class R>
  friend Vec2<R>
  operator*(const R , const Vec2<R>& );
  
  // Operator Vec2/Real
  Vec2 operator/(const Real ) const;
  
  // Operator -Vec2
  Vec2 operator-();
  
  
  /* Equality and relational operators */
  
  // Operator Vec2 == Vec2
  bool operator==(const Vec2& ) const;
  
  // Operator Vec2 != Vec2
  bool operator!=(const Vec2& ) const;
  
  // Operator Vec2 < Vec2
  bool operator<(const Vec2& ) const;
  
  
  /* I/O stream operators */
  
  // Operator <<
  template <class R>
  friend std::ostream&
  operator<<(std::ostream& , const Vec2<R>& );
  
  // Operator >>
  template <class R>
  friend std::istream&
  operator>>(std::istream& , Vec2<R>& );
  
  
  /* Norms */
  
  // Norm (aka l-2 norm, euclidean norm)
  Real norm() const;
  
  // Square norm
  Real sqnorm() const;
  
  // Infinite norm (aka l-infinity norm)
  Real inftNorm() const;
  
  // Normalization
  Vec2& normalize();
  
  
  /* Constants */
  
  // Null Vec2
  static const Vec2 null();
  
  // Size
  static int size();
  
  
private:
  
  Real a[2];
  
}; // class Vec2



// -----------------------------
//  
//  Vec2 class-related methods.
//  
//  David Bourguignon.
//  
// -----------------------------

/* Dot and cross products */

// Dot product
template <class Real>
Real dot(const Vec2<Real>& , const Vec2<Real>& );

// Cross product
template <class Real>
Real cross(const Vec2<Real>& , const Vec2<Real>& );


// Distance between p1 and p2
template <class Real>
Real dist(const Vec2<Real>& p1, const Vec2<Real>& p2);

// Square distance between p1 and p2
template <class Real>
Real sqdist(const Vec2<Real>& p1, const Vec2<Real>& p2);

// Cosinus of the (v1,v2) angle
template <class Real>
Real cosAng(const Vec2<Real>& v1, const Vec2<Real>& v2);










// -------------------------------------------------------------
// 
//   D E F I N I T I O N   O F   I N L I N E D   M E T H O D S 
// 
// -------------------------------------------------------------

template <class Real>
inline
Vec2<Real>::
Vec2()
{}

template <class Real>
inline
Vec2<Real>::
Vec2(const Real x0, const Real y0)
{
  a[0] = x0; a[1] = y0;
}





template <class Real>
inline const Real&
Vec2<Real>::
x() const 
{
  return a[0];
}

template <class Real>
inline const Real&
Vec2<Real>::
y() const 
{
  return a[1];
}

template <class Real>
inline const Real&
Vec2<Real>::
operator[](int i) const
{
  return a[i];
}





template <class Real>
inline Vec2<Real>&
Vec2<Real>::
setx(const Real x0)
{
  a[0] = x0;
  return *this;
}

template <class Real>
inline Vec2<Real>&
Vec2<Real>::
sety(const Real y0)
{
  a[1] = y0;
  return *this;
}

template <class Real>
inline Real&
Vec2<Real>::
operator[](int i)
{
  return a[i];
}





template <class Real>
inline Vec2<Real>&
Vec2<Real>::
operator+=(const Vec2& v)
{
  a[0] += v.x();
  a[1] += v.y();
  return *this;
}

template <class Real>
inline Vec2<Real>&
Vec2<Real>::
operator-=(const Vec2& v)
{
  a[0] -= v.x();
  a[1] -= v.y();
  return *this;
}

template <class Real>
inline Vec2<Real>&
Vec2<Real>::
operator*=(const Real k)
{
  a[0] *= k;
  a[1] *= k;
  return *this;
}

template <class Real>
inline Vec2<Real>&
Vec2<Real>::
operator/=(const Real k)
{
  (*this) *= 1.0/k;
  return *this;
}





template <class Real>
inline Vec2<Real>
Vec2<Real>::
operator+(const Vec2& v) const
{
  return Vec2( a[0]+v.x(), a[1]+v.y() );
}

template <class Real>
inline Vec2<Real>
Vec2<Real>::
operator-(const Vec2& v) const
{
  return Vec2( a[0]-v.x(), a[1]-v.y() );
}

template <class Real>
inline Vec2<Real>
Vec2<Real>::
operator*(const Real k) const
{
  return Vec2(a[0]*k, a[1]*k);
}

template <class Real>
inline Vec2<Real>
operator*(const Real k, const Vec2<Real>& v)
{
  return v*k;
}

template <class Real>
inline Vec2<Real>
Vec2<Real>::
operator/(const Real k) const
{
  return (*this)*(1.0/k);
}

template <class Real>
inline Vec2<Real>
Vec2<Real>::
operator-()
{
  return Vec2( -x(), -y() );
}





template <class Real>
inline bool
Vec2<Real>::
operator==(const Vec2& v) const
{
  return ( (v.x() == a[0]) && (v.y() == a[1]) );
}

template <class Real>
inline bool
Vec2<Real>::
operator!=(const Vec2& v) const
{
  return !operator==(v);
}

template <class Real>
inline bool
Vec2<Real>::
operator<(const Vec2& v) const
{
  return ( (a[0] < v.x()) && (a[1] < v.y()) );
}





template <class Real>
inline std::ostream&
operator<<(std::ostream& s, const Vec2<Real>& v)
{
  return s << v.x() << " " << v.y();
}

template <class Real>
inline std::istream&
operator>>(std::istream& s, Vec2<Real>& v)
{
  Real x0 = 0.0;
  Real y0 = 0.0;
  
  s >> x0 >> y0;
  v = Vec2<Real>(x0, y0);
  return s;
}





template <class Real>
inline Real
Vec2<Real>::
norm() const 
{
  return Numerics<Real>::sqroot( a[0]*a[0] + a[1]*a[1] );
}

template <class Real>
inline Real
Vec2<Real>::
sqnorm() const
{
  return ( a[0]*a[0] + a[1]*a[1] );
}

template <class Real>
inline Real
Vec2<Real>::
inftNorm() const
{
  Real x0 = a[0];
  Real y0 = a[1];
  
  if ( x0 < 0.0 ) x0 = -x0;
  if ( y0 < 0.0 ) y0 = -y0;
  
  if ( x0 < y0 )
    return y0;
  else
    return x0;
}

template <class Real>
inline Vec2<Real>&
Vec2<Real>::
normalize()
{
  (*this) *= 1.0/norm();
  return *this;
}





template <class Real>
inline const Vec2<Real>
Vec2<Real>::
null()
{
  return Vec2( Real(0.0), Real(0.0) );
}

template <class Real>
inline int
Vec2<Real>::
size()
{
  return 2;
}





template <class Real>
inline Real
dot(const Vec2<Real>& v1, const Vec2<Real>& v2)
{
  return ( v1.x()*v2.x() + v1.y()*v2.y() );
}

template <class Real>
inline Real
cross(const Vec2<Real>& v1, const Vec2<Real>& v2)
{
  return ( v1.x()*v2.y() - v1.y()*v2.x() );
}





template <class Real>
inline Real
dist(const Vec2<Real>& p1, const Vec2<Real>& p2)
{
  return (p1 - p2).norm();
}

template <class Real>
inline Real
sqdist(const Vec2<Real>& p1, const Vec2<Real>& p2)
{
  return (p1 - p2).sqnorm();
}

template <class Real>
inline Real
cosAng(const Vec2<Real>& v1, const Vec2<Real>& v2)
{
  return dot(v1,v2)/Numerics<Real>::sqroot( v1.sqnorm()*v2.sqnorm() );
}



#endif // VEC2_H
