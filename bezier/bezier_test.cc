#include <GL/glui.h>
#include "trackball.h"
#include "widgets.h"
#include "bezier_curve.h"
#include "bezier_surface.h"

/* Typedefs */
#if FLOAT
typedef GLfloat                    real;
#else
typedef GLdouble                   real;
#endif
typedef Vec3<real>                 vec3;
typedef Quat<real>                 quat;
typedef Trackball<real>            trackball;
typedef Bezier_Curve<real, vec3>   bezier_curve;
typedef Bezier_Surface<real, vec3> bezier_surface;

/* Data */
trackball tb;
int main_window;
GLuint axes_list;
int white_background, show_ball, show_axes, curves, surfaces, derivatives;
GLUI* glui;

/* Test data */
vec3 Q, Q_prime, Q_second;
const int order_u = 4;
const int order_v = 4;
real ctrlpoints_curve[order_u][3] = {
  {-4.0, -4.0, 0.0}, {-2.0, 4.0, 0.0}, {2.0, -4.0, 0.0}, {4.0, 4.0, 0.0}
};
real ctrlpoints_surface[order_u][order_v][3] = {
  {{-1.5, -1.5, 4.0}, {-0.5, -1.5, 2.0}, {0.5, -1.5, -1.0}, {1.5, -1.5, 2.0}}, 
  {{-1.5, -0.5, 1.0}, {-0.5, -0.5, 3.0}, {0.5, -0.5, 0.0}, {1.5, -0.5, -1.0}}, 
  {{-1.5, 0.5, 4.0}, {-0.5, 0.5, 0.0}, {0.5, 0.5, 3.0}, {1.5, 0.5, 4.0}}, 
  {{-1.5, 1.5, -2.0}, {-0.5, 1.5, -2.0}, {0.5, 1.5, 0.0}, {1.5, 1.5, -1.0}}
};
bezier_curve bc1;
bezier_curve bc2(3);
bezier_surface bs1;
bezier_surface bs2(3, 3);

void reshape(int w, int h) {
  glViewport( 0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h) );
  GLdouble aspect = static_cast<GLfloat>(w)/ static_cast<GLfloat>(h);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, aspect, 1.0e-2, 1.0e+2);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  tb.reshape(w,h);
}

void display() {
  static real m[4][4];
  if (white_background)
    glClearColor(1.0, 1.0, 1.0, 1.0);
  else
    glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  tb.writeOpenGLTransfMatrix(m);
  
  glPushMatrix();
#if FLOAT
  glMultMatrixf(&m[0][0]);
#else
  glMultMatrixd(&m[0][0]);
#endif
  if (white_background)
    glColor3f(0.0, 0.0, 0.0);
  else
    glColor3f(1.0, 1.0, 1.0);
  if (show_ball)
    glutWireSphere(1.0, 10, 10);
  if (show_axes)
    glCallList(axes_list);
  
  // Test
  glPushAttrib(GL_EVAL_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_POINT_BIT);
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_3);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(1.5);
  glPointSize(5.0);
  
  const GLint nsteps_u = 20;
  const GLint nsteps_v = 10;
  if (curves) {
    glColor3f(1.0, 0.0, 0.0);
    bc1.draw(nsteps_u);
    glColor3f(1.0, 0.0, 1.0);
    bc1.drawControlPoints();
    
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < nsteps_u + 1; i++) {
      bc2.evaluate(i*(1.0/nsteps_u), Q);
#if FLOAT
      glVertex3fv(&Q[0]);
#else
      glVertex3dv(&Q[0]);
#endif
    }
    glEnd();
    glColor3f(1.0, 0.0, 0.0);
    bc2.drawControlPoints();
    if (derivatives) {
      glBegin(GL_LINES);
      for (int i = 0; i < nsteps_u + 1; i++) {
	bc2.evaluate(i*(1.0/nsteps_u), Q);
        bc2.evaluateDerivative(1, i*(1.0/nsteps_u), Q_prime);
	bc2.evaluateDerivative(2, i*(1.0/nsteps_u), Q_second);
	glColor3f(0.0, 0.9, 0.1);
#if FLOAT
        glVertex3fv(&Q[0]);
        glVertex3f(Q[0] + Q_prime[0], Q[1] + Q_prime[1], Q[2] + Q_prime[2]);
#else
	glVertex3dv(&Q[0]);
        glVertex3d(Q[0] + Q_prime[0], Q[1] + Q_prime[1], Q[2] + Q_prime[2]);
#endif
	glColor3f(0.0, 0.5, 0.5);
#if FLOAT
        glVertex3fv(&Q[0]);
        glVertex3f(Q[0] + Q_second[0], Q[1] + Q_second[1], Q[2] + Q_second[2]);
#else
	glVertex3dv(&Q[0]);
        glVertex3d(Q[0] + Q_second[0], Q[1] + Q_second[1], Q[2] + Q_second[2]);
#endif
      }
      glEnd();
    }
  }
  if (surfaces) {
    glColor3f(0.0, 0.0, 1.0);
    bs1.draw(nsteps_u, nsteps_v);
    glColor3f(0.0, 1.0, 1.0);
    bs1.drawControlPoints();
    
    glColor3f(0.0, 1.0, 1.0);
    for (int j = 0; j < nsteps_v + 1; j++) {
      glBegin(GL_LINE_STRIP);
      for (int i = 0; i < nsteps_u + 1; i++) {
        bs2.evaluate(i*(1.0/nsteps_u), j*(1.0/nsteps_v), Q);
#if FLOAT
        glVertex3fv(&Q[0]);
#else
	glVertex3dv(&Q[0]);
#endif
      }
      glEnd();
    }
    for (int i = 0; i < nsteps_u + 1; i++) {
      glBegin(GL_LINE_STRIP);
      for (int j = 0; j < nsteps_v + 1; j++) {
        bs2.evaluate(i*(1.0/nsteps_u), j*(1.0/nsteps_v), Q);
#if FLOAT
        glVertex3fv(&Q[0]);
#else
	glVertex3dv(&Q[0]);
#endif
      }
      glEnd();
    }
    glColor3f(0.0, 0.0, 1.0);
    bs2.drawControlPoints();
  }
  
  glPopAttrib();
  glPopMatrix();
  
  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  switch ( key ) {
  case 'h':
    printf("\n");
    printf("\to\tback to Origin\n");
    printf("\t\tq\tQuit\n");
    printf("\t\th\tdisplay Help\n");
    printf("\n");
    break;
  case 'o':
    tb.reinitializeTransf();
    glutPostRedisplay();
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  default:
    fprintf(stderr, "Error: unknown option !\n");
    break;
  }
}

void mouse(int button, int state, int x, int y) {
  switch (button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN)
      tb.startRotation(x,y);
    else
      tb.stopRotation();
    break;
  case GLUT_MIDDLE_BUTTON:
    if (state == GLUT_DOWN)
      tb.startTranslationZ(x,y);
    else
      tb.stopTranslationZ();
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN)
      tb.startTranslationXY(x,y);
    else
      tb.stopTranslationXY();
    break;
  }
}

void motion(int x, int y) {
  tb.move(x,y);
  glutPostRedisplay();
}

void buildGLUIinterface(int gfx_window) {
  glui = GLUI_Master.create_glui("GUI");
  GLUI_Panel* options_panel = glui->add_panel("Options");
  glui->add_checkbox_to_panel(options_panel, "White background",
			      &white_background);
  glui->add_checkbox_to_panel(options_panel, "Show axes", &show_axes);
  glui->add_checkbox_to_panel(options_panel, "Display curves", &curves);
  glui->add_checkbox_to_panel(options_panel, "Display surfaces", &surfaces);
  glui->add_checkbox_to_panel(options_panel, "Display derivatives",
			      &derivatives);
  glui->set_main_gfx_window(gfx_window);
  GLUI_Master.set_glutIdleFunc(0);
}

void init(char* name) {
  main_window = glutCreateWindow(name);
  glutInitWindowSize(300, 300);
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  buildGLUIinterface(main_window);
  
  axes_list = glGenLists(1);
  glNewList(axes_list, GL_COMPILE);
  axes();
  glEndList();
  
  quat qInit1 = quat( vec3(1.0, 0.0, 0.0), -M_PI_4/4 );
  quat qInit2 = quat( vec3(0.0, 1.0, 0.0), M_PI_4/2 );
  quat qInit  = qInit1*qInit2;
  vec3 vInit  = vec3(0.0, 0.0, -10.0);
  tb = trackball(qInit, vInit);
  
  white_background = false;
  show_ball = false;
  show_axes = false;
  
  for (int i = 0; i < order_u; i++) {
    bc1.V.push_back(vec3(ctrlpoints_curve[i][0], ctrlpoints_curve[i][1],
		         ctrlpoints_curve[i][2]));
  }
  for (int i = 0; i < order_u; i++) {
    bc2.V[i] = vec3(ctrlpoints_curve[i][0], ctrlpoints_curve[i][1],
		    ctrlpoints_curve[i][2])
             + vec3(0.0, 0.0, 1.0);
  }
  for (int i = 0; i < order_u; i++) {
    for (int j = 0; j < order_v; j++) {
      bs1.V.push_back(vec3(ctrlpoints_surface[i][j][0],
			   ctrlpoints_surface[i][j][1],
			   ctrlpoints_surface[i][j][2]));
    }
  }
  bs1.order_u = order_u;
  bs1.order_v = order_v;
  for (int i = 0; i < order_u; i++) {
    for (int j = 0; j < order_v; j++) {
      bs2.setControlPoint(i, j, vec3(ctrlpoints_surface[i][j][0],
			             ctrlpoints_surface[i][j][1],
			             ctrlpoints_surface[i][j][2])
	                      + vec3(0.0, 0.0, 1.0));
    }
  }
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  init("Viewer");
  glutMainLoop();
  return 0;
}
