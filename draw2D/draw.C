#include "input.h"
#include "drawing.h"

using namespace std;

/* Data */
Input in;
Drawing D;
int win;
GLfloat bg_color[4] =  {1.0, 1.0, 1.0, 1.0};
GLfloat stroke_color[4] = {0.0, 0.0, 0.0, 1.0};
bool changed_color = false;
bool is_read = false;
bool is_written = false;
enum mousemode {DRAW};

/* Functions declaration */
void reshape (int w, int h);
void display();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

/* Functions definition */
void reshape(int w, int h) {
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  GLint v[4];
  glGetIntegerv(GL_VIEWPORT, v);
  D.setViewport(v);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, static_cast<GLdouble>(w), 0.0, static_cast<GLdouble>(h));
  /*
  if (w <= h)
    glOrtho(-5.0, 5.0,
    -5.0*static_cast<GLdouble>(h)/static_cast<GLdouble>(w),
    5.0*static_cast<GLdouble>(h)/static_cast<GLdouble>(w), -5.0, 5.0);
  else
    glOrtho(-5.0*static_cast<GLdouble>(w)/static_cast<GLdouble>(h),
             5.0*static_cast<GLdouble>(w)/static_cast<GLdouble>(h),
	     -5.0, 5.0, -5.0, 5.0);
  */
  D.setFrustumSize(static_cast<GLdouble>(w) - 0.0,
		   static_cast<GLdouble>(h) - 0.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glPointSize(4.0);
  glLineWidth(1.0);
  glColor4fv(stroke_color);
  in.draw2D();
  D.draw();
  glutSwapBuffers();
  glFlush();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'c':
    if (!changed_color) {
      changed_color = true;
      D.setBgColor(0.0, 0.0, 0.0, 1.0);
      stroke_color[0] = 1.0; stroke_color[1] = 1.0;
      stroke_color[2] = 1.0; stroke_color[3] = 1.0;
    }
    else {
      changed_color = false;
      D.setBgColor(1.0, 1.0, 1.0, 1.0);
      stroke_color[0] = 0.0; stroke_color[1] = 0.0;
      stroke_color[2] = 0.0; stroke_color[3] = 1.0;
    }
    glutPostRedisplay();
    break;
  case 'h':
    printf("\n");
    printf("H e l p !\n");
    printf("c\tchange Color\n");
    printf("h\tdisplay Help\n");
    printf("l\tLoad (read) stroke2D data\n");
    printf("m\tMark (write) stroke2D data\n");
    printf("q\tQuit\n");
    printf("r\tRead input data\n");
    printf("w\tWrite input data switch\n");
    printf("\n");
    break;
  case 'l':
    D.read("strokes.dat");
    break;
  case 'm':
    D.write("strokes.dat");
    break;
  case 'r':
    in.read("input.dat");
    D.addStroke(Stroke2D(in));
    //in.clear();
    break;
  case 'w':
    if (is_written) {
      is_read = true;
      is_written = false;
    }
    else {
      is_read = false;
      is_written = true;
    }
    cout << "R " << is_read << " W " << is_written << endl;
    break;
  case 'q':
  case 27:
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
    if (state == GLUT_DOWN) {
      D.setMouseMode(DRAW);
    }
    else {
      D.addStroke(Stroke2D(in));
      if (is_written) {
	in.write("input.dat");
      }
      in.clear();
    }
    break;
  case GLUT_MIDDLE_BUTTON:
  case GLUT_RIGHT_BUTTON:
    break;
  }
  glutPostRedisplay();
}

void motion(int x, int y) {
  if (D.mouseMode() == DRAW)
    in.addPoint2D(x, D.getViewport(3) - y - 1);
  glutPostRedisplay();
}

/* Main program */
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
  win = glutCreateWindow("T h e   D r a w i n g   B o a r d");
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  D.setBgColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
  in.point_size = 1.0;
  glutMainLoop();
  return 0;
}
