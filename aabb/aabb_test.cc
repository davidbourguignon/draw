#include <GL/glut.h>
#include "aabb.h"

using namespace std;

/* Types */
typedef GLdouble         real;
typedef Vec2<real>       vec2;
typedef AABB<real, vec2> bounding_box;

/* Data */
GLint viewport_win[4];
bounding_box box;
std::vector<bounding_box> boxes;

/* Functions declaration */
void reshape (int w, int h);
void display();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

/* Functions definition */
void reshape(int w, int h) {
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  glGetIntegerv(GL_VIEWPORT, viewport_win);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, static_cast<GLdouble>(w), 0.0, static_cast<GLdouble>(h));
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  for (int i = 0; i < boxes.size(); i++) {
    const GLfloat color
      = static_cast<GLfloat>(i+1)/static_cast<GLfloat>(boxes.size()+1);
    glColor3f(color, 0.0, 1.0 - color);
    boxes[i].draw();
  }
  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'h':
    printf("\n");
    printf("H e l p !\n");
    printf("h\tdisplay Help\n");
    printf("q\tQuit\n");
    printf("\n");
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
  static bool first_done = false;
  static bool last_done = false;
  static std::vector<vec2> points;
  if (state == GLUT_DOWN) {
    GLint y_OpenGL = viewport_win[3] - 1 - y;
    if (!first_done) {
      points.push_back(vec2(x, y_OpenGL));
      first_done = true;
    }
    else if (!last_done) {
      points.push_back(vec2(x, y_OpenGL));
      last_done = true;
    }
  }
  else {
    if (first_done && last_done) {
      box = bounding_box(points);
      points.clear();
      std::vector<int> indices;
      for (int i = 0; i < boxes.size(); i++) {
	if (box.isIntersectedBy(boxes[i])) {
	  indices.push_back(i);
	}
      }
      if (!indices.empty()) {
	cout << "Is intersected by: ";
#if 0
	std::copy(indices.begin(), indices.end(),
		  std::ostream_iterator<int>(cout, " "));
#else
	for (int i = 0; i < indices.size(); i++) {
	  cout << indices[i] << " ";
	}
#endif
        cout << endl;
      }
      else {
	cout << "No intersection" << endl;
      }
      boxes.push_back(box);
      first_done = last_done = false;
    }
  }
  glutPostRedisplay();
}

/* Main program */
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutCreateWindow("aabb_test.cc");
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  boxes.push_back(bounding_box(vec2(100, 100), vec2(200, 200)));
  glutMainLoop();
  return 0;
}
