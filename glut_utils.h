#ifndef GLUT_UTILS_H
#define GLUT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <GL/glut.h>

typedef struct _GLUTWindow GLUTWindow;
typedef struct _GLUTDevices GLUTDevices;

struct _GLUTWindow {
  int id;
  int width, height;
  int border; /* For sub window */
};
struct _GLUTDevices {
  int has_tablet; /* GLUT assume window system always has mouse and keyboard. */
  int num_mouse_buttons, num_tablet_buttons;
};

__inline__ void glut_window_set_size(GLUTWindow *win, int w, int h);
extern     void glut_devices_check  (GLUTDevices *devs);

__inline__ void
glut_window_set_size(GLUTWindow *win, int w, int h) {
  win->width = w;
  win->height = h;
}

#ifdef __cplusplus
}
#endif

#endif /* GLUT_UTILS_H */
