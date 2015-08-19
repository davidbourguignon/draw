#ifndef OPENGLUTILS_H
#define OPENGLUTILS_H

#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "vec3.h"

void nullV(GLdouble v[4]);
void nullM(GLdouble m[16]);
void identityM(GLdouble m[16]);
GLboolean invert(const GLdouble src[16], GLdouble inverse[16]);
void multMV(const GLdouble a[16], const GLdouble x[4], GLdouble b[4]);
void multMM(const GLdouble m1[16], const GLdouble m2[16], GLdouble m3[16]);
void printM(const GLdouble m[16]);
void printV(const GLdouble v[4]);
void writeViewVector(const GLdouble mv_matrix[16],
		     Vec3<GLdouble>& view);
void writeCenterOfProjection(const GLdouble mv_matrix[16],
			     Vec3<GLdouble>& center);

#endif // OPENGLUTILS_H
