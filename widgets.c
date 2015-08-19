#include "widgets.h"

void
axes() {
  glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
  
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  
  glLineWidth(2.0);
  glBegin(GL_LINES);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 1.0);
  glEnd();
  
  glColor3f(1.0, 1.0, 0.0);
  glutSolidCube(0.075);
  
  glPushMatrix();
  glTranslatef(1.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 0.0);
  glutSolidCube(0.075);
  glPopMatrix();
  
  glPushMatrix();
  glTranslatef(0.0, 1.0, 0.0);
  glColor3f(0.0, 1.0, 0.0);
  glutSolidCube(0.075);
  glPopMatrix();
  
  glPushMatrix();
  glTranslatef(0.0, 0.0, 1.0);
  glColor3f(0.0, 0.0, 1.0);
  glutSolidCube(0.075);
  glPopMatrix();
  
  glPopAttrib();
}

void
box(const GLfloat width, const GLfloat length, const GLfloat height) {
  GLfloat x = 0.5*width;
  GLfloat y = 0.5*height;
  GLfloat z = 0.5*length;
  
  glBegin(GL_QUAD_STRIP);
  glVertex3f(-x, -y, z);
  glVertex3f(x, -y, z);
  glVertex3f(-x, y, z);
  glVertex3f(x, y, z);
  glVertex3f(-x, y, -z);
  glVertex3f(x, y, -z);
  glVertex3f(-x, -y, -z);
  glVertex3f(x, -y, -z);
  glEnd();
  
  glBegin(GL_QUAD_STRIP);
  glVertex3f(-x, y, z);
  glVertex3f(-x, y, -z);
  glVertex3f(-x, -y, z);
  glVertex3f(-x, -y, -z);
  glVertex3f(x, -y, z);
  glVertex3f(x, -y, -z);
  glVertex3f(x, y, z);
  glVertex3f(x, y, -z);
  glEnd();
}

void
grid(const GLfloat width, const GLfloat length,
     const GLint rows, const GLint columns) {
  GLfloat x = 0.5*width;
  GLfloat z = 0.5*length;
  GLfloat step_x = width/columns;
  GLfloat step_z = length/rows;
  int c, r;
  for (c = 0; c < columns; c++) {
    GLfloat x_prev = -x +  c   *step_x;
    GLfloat x_next = -x + (c+1)*step_x;
    glBegin(GL_QUAD_STRIP);
    for (r = 0; r <= rows; r++) {
      GLfloat z_curr = z - r*step_z;
      glVertex3f(x_prev, 0.0, z_curr);
      glVertex3f(x_next, 0.0, z_curr);
    }
    glEnd();
  }
}

void
teapot(GLdouble size) {
  /* Light */
  const GLfloat lKa[4] = {0.25, 0.15, 0.15, 1.0};
  const GLfloat lKd[4] = {1.00, 0.85, 0.85, 1.0};
  const GLfloat lKs[4] = {1.00, 0.99, 0.99, 1.0};
  
  /* Nate Robins' Polished_Copper material */
  const GLfloat mKa[4] = {0.229500, 0.088250, 0.027500, 1.000000};
  const GLfloat mKd[4] = {0.550800, 0.211800, 0.066000, 1.000000};
  const GLfloat mKs[4] = {0.580594, 0.223257, 0.069570, 1.000000};
  const GLfloat S      = 51.200001;
  
  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
  
  glLightfv(GL_LIGHT0, GL_AMBIENT, lKa);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lKd);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lKs);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mKa);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mKd);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mKs);
  glMaterialf(GL_FRONT, GL_SHININESS, S);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glutSolidTeapot(size);
  
  glPopAttrib();
}

