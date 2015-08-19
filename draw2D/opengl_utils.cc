#include "opengl_utils.h"

void
nullV(GLdouble v[4]) {
  v[0] = 0.0;
  v[1] = 0.0;
  v[2] = 0.0;
  v[3] = 0.0;
}

void
nullM(GLdouble m[16]) {
  m[0+4*0] = 0.0; m[0+4*1] = 0.0; m[0+4*2] = 0.0; m[0+4*3] = 0.0;
  m[1+4*0] = 0.0; m[1+4*1] = 0.0; m[1+4*2] = 0.0; m[1+4*3] = 0.0;
  m[2+4*0] = 0.0; m[2+4*1] = 0.0; m[2+4*2] = 0.0; m[2+4*3] = 0.0;
  m[3+4*0] = 0.0; m[3+4*1] = 0.0; m[3+4*2] = 0.0; m[3+4*3] = 0.0;
}

void
identityM(GLdouble m[16]) {
  m[0+4*0] = 1.0; m[0+4*1] = 0.0; m[0+4*2] = 0.0; m[0+4*3] = 0.0;
  m[1+4*0] = 0.0; m[1+4*1] = 1.0; m[1+4*2] = 0.0; m[1+4*3] = 0.0;
  m[2+4*0] = 0.0; m[2+4*1] = 0.0; m[2+4*2] = 1.0; m[2+4*3] = 0.0;
  m[3+4*0] = 0.0; m[3+4*1] = 0.0; m[3+4*2] = 0.0; m[3+4*3] = 1.0;
}

GLboolean
invert(const GLdouble src[16], GLdouble inverse[16]) {
  GLdouble t;
  GLint i, j, k, swap;
  GLdouble tmp[4][4];
  
  identityM(inverse);
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      tmp[i][j] = src[i*4+j];
    }
  }
  for (i = 0; i < 4; i++) {
    /* Look for largest element in column. */
    swap = i;
    for (j = i + 1; j < 4; j++) {
      if (fabs(tmp[j][i]) > fabs(tmp[i][i])) {
	swap = j;
      }
    }
    if (swap != i) {
      /* Swap rows. */
      for (k = 0; k < 4; k++) {
	t = tmp[i][k];
	tmp[i][k] = tmp[swap][k];
        tmp[swap][k] = t;
        
        t = inverse[i*4+k];
        inverse[i*4+k] = inverse[swap*4+k];
        inverse[swap*4+k] = t;
      }
    }
    if (tmp[i][i] == 0) {
      /* No non-zero pivot. The matrix is singular, which shouldn't happen.
         This means the user gave us a bad matrix. */
      return GL_FALSE;
    }
    t = tmp[i][i];
    for (k = 0; k < 4; k++) {
      tmp[i][k] /= t;
      inverse[i*4+k] /= t;
    }
    for (j = 0; j < 4; j++) {
      if (j != i) {
	t = tmp[j][i];
	for (k = 0; k < 4; k++) {
	  tmp[j][k] -= tmp[i][k]*t;
          inverse[j*4+k] -= inverse[i*4+k]*t;
	}
      }
    }
  }
  return GL_TRUE;
}

void
multMV(const GLdouble a[16], const GLdouble x[4], GLdouble b[4]) {
  GLint i, j;
  
  nullV(b);
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      b[i] += a[i+4*j] * x[j];
    }
  }
}

void
multMM(const GLdouble m1[16], const GLdouble m2[16], GLdouble m3[16]) {
  GLint i, j, k;
  
  nullM(m3);
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      for (k = 0; k < 4; k++) {
	m3[i+4*j] += m1[i+4*k] * m2[k+4*j];
      }
    }
  }
}

void
printM(const GLdouble m[4][4]) {
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[0][0], m[1][0], m[2][0], m[3][0]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[0][1], m[1][1], m[2][1], m[3][1]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[0][2], m[1][2], m[2][2], m[3][2]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[0][3], m[1][3], m[2][3], m[3][3]);
  fprintf(stderr, "\n");
}

void
printM(const GLdouble m[16]) {
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[0], m[4],  m[8], m[12]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[1], m[5],  m[9], m[13]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[2], m[6], m[10], m[14]);
  fprintf(stderr, "%+5.5f\t%+5.5f\t%+5.5f\t%+5.5f\n",
	  m[3], m[7], m[11], m[15]);
  fprintf(stderr, "\n");
}

void printV(const GLdouble v[4]) {
  fprintf(stderr, "%+5.5f\n", v[0]);
  fprintf(stderr, "%+5.5f\n", v[1]);
  fprintf(stderr, "%+5.5f\n", v[2]);
  fprintf(stderr, "%+5.5f\n", v[3]);
  fprintf(stderr, "\n");
}

void
writeViewVector(const GLdouble mv_matrix[16], Vec3<GLdouble>& view) {
  GLdouble mv_matrix_inv[16];
  if (!invert(mv_matrix, mv_matrix_inv)) {
    assert(false);
  }
  GLdouble z[4] = {0.0, 0.0, 1.0, 1.0};
  GLdouble O[4] = {0.0, 0.0, 0.0, 1.0};
  GLdouble z_transf[4];
  GLdouble O_transf[4];
  multMV(mv_matrix_inv, z, z_transf);
  multMV(mv_matrix_inv, O, O_transf);
  view.setx(O_transf[0] - z_transf[0]);
  view.sety(O_transf[1] - z_transf[1]);
  view.setz(O_transf[2] - z_transf[2]);
}

void
writeCenterOfProjection(const GLdouble mv_matrix[16], Vec3<GLdouble>& center) {
  GLdouble mv_matrix_inv[16];
  if (!invert(mv_matrix, mv_matrix_inv)) {
    assert(false);
  }
  GLdouble O[4] = {0.0, 0.0, 0.0, 1.0};
  GLdouble O_transf[4];
  multMV(mv_matrix_inv, O, O_transf);
  center.setx(O_transf[0]);
  center.sety(O_transf[1]);
  center.setz(O_transf[2]);
}
