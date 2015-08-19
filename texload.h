#ifndef TEXLOAD_H
#define TEXLOAD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <GL/gl.h>

GLubyte *read_alpha_texture(char *name, int *width, int *height);
GLubyte *read_rgb_texture(char *name, int *width, int *height);
GLubyte *read_rgba_texture(char *name, int *width, int *height);

#if defined(__cplusplus) || defined(c_plusplus)
} /* Close extern "C" declaration. */
#endif

#endif /* TEXLOAD_H */
