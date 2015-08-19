#ifndef AABB_H
#define AABB_H

#include <stdio.h>
#include <fstream>
#include <vector>
#include <GL/glu.h>
#include "vec2.h"
#include "vec3.h"

template < class Real, class Vec = Vec2<Real> >
class AABB {
public:
  typedef Real real;
  typedef Vec  vec;
  
  AABB(); // Be careful: contains random values!
  AABB(const Vec& point_min, const Vec& point_max);
  AABB(const std::vector<Vec>& points);
  AABB(const AABB& box);
  void insert(const Vec& point);
  void insert(const AABB& box);
  bool isIntersectedBy(const AABB& box) const;
  void draw() const;
  void read(std::ifstream& file_in);
  void write(std::ofstream& file_out) const;
  
  Vec min, max;
};

#if 0
template < class Real, class Vec = Vec2<Real> >
class AABB_Tree {};
#endif

/*
 *  Definition of inlined methods
 */

template <class Real, class Vec>
inline AABB<Real, Vec>::
AABB()
  : min(), max() {}

template <class Real, class Vec>
inline AABB<Real, Vec>::
AABB(const Vec& point_min, const Vec& point_max)
  : min(point_min), max(point_max) {}

template <class Real, class Vec>
inline AABB<Real, Vec>::
AABB(const std::vector<Vec>& points)
  : min(points[0]), max(points[0]) {
  for (int i = 1; i < points.size(); i++) {
    insert(points[i]);
  }
}

template <class Real, class Vec>
inline AABB<Real, Vec>::
AABB(const AABB& box)
  : min(box.min), max(box.max) {}

template <class Real, class Vec>
inline void AABB<Real, Vec>::
insert(const Vec& point) {
  for (int i = 0; i < Vec::size(); i++) {
    const Real point_i = point[i];
    if (point_i < min[i]) {
      min[i] = point_i;
    }
    else if (point_i > max[i]) {
      max[i] = point_i;
    }
  }
}

template <class Real, class Vec>
inline void AABB<Real, Vec>::
insert(const AABB& box) {
  for (int i = 0; i < Vec::size(); i++) {
    const Real box_min_i = box.min[i];
    if (box_min_i < min[i]) {
      min[i] = box_min_i;
    }
    const Real box_max_i = box.max[i];
    if (box_max_i > max[i]) {
      max[i] = box_max_i;
    }
  }
}

template <class Real, class Vec>
inline bool AABB<Real, Vec>::
isIntersectedBy(const AABB& box) const {
  Vec box_min = box.min;
  Vec box_max = box.max;
  bool answer = false;
  for (int i = 0; i < Vec::size(); i++) {
    answer = answer || (min[i] > box_max[i] || max[i] < box_min[i]);
  }
  return !answer;
}

template <class Real, class Vec>
inline void AABB<Real, Vec>::
draw() const {}

template <>
inline void AABB< double, Vec2<double> >::
draw() const {
  glBegin(GL_LINE_LOOP);
  glVertex2dv(&min[0]);
  glVertex2d(max[0], min[1]);
  glVertex2dv(&max[0]);
  glVertex2d(min[0], max[1]);
  glEnd();
}

template <>
inline void AABB< double, Vec3<double> >::
draw() const {
  glBegin(GL_LINE_LOOP);
  glVertex3dv(&min[0]);
  glVertex3d(max[0], min[1], min[2]);
  glVertex3d(max[0], max[1], min[2]);
  glVertex3d(min[0], max[1], min[2]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  glVertex3d(min[0], min[1], max[2]);
  glVertex3d(max[0], min[1], max[2]);
  glVertex3dv(&max[0]);
  glVertex3d(min[0], max[1], max[2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3dv(&min[0]);
  glVertex3d(min[0], min[1], max[2]);
  glVertex3d(max[0], min[1], min[2]);
  glVertex3d(max[0], min[1], max[2]);
  glVertex3d(max[0], max[1], min[2]);
  glVertex3dv(&max[0]);
  glVertex3d(min[0], max[1], min[2]);
  glVertex3d(min[0], max[1], max[2]);
  glEnd();
}

template <class Real, class Vec>
inline void AABB<Real, Vec>::
read(std::ifstream& file_in) {}

template <>
inline void AABB< double, Vec3<double> >::
read(std::ifstream& file_in) {
  char line[256];
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf %lf %lf", &min[0], &min[1], &min[2]);
  file_in.getline(line, 256, '\n');
  sscanf(line, "%lf %lf %lf", &max[0], &max[1], &max[2]);
}

template <class Real, class Vec>
inline void AABB<Real, Vec>::
write(std::ofstream& file_out) const {
  file_out << min << endl;
  file_out << max << endl;
}

#endif // AABB_H
