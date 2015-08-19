#ifndef WIDGETS_H
#define WIDGETS_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <GL/glut.h>

void axes();
void box(const GLfloat width, const GLfloat length, const GLfloat height);
void grid(const GLfloat width, const GLfloat length,
	  const GLint rows, const GLint columns);
void teapot(GLdouble size);

#if defined(__cplusplus) || defined(c_plusplus)
} /* Close extern "C" declaration. */
#endif

#endif /* WIDGETS_H */
