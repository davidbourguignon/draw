#include <glut_utils.h>

void
glut_devices_check(GLUTDevices *devs) {
  int h_ta;
  int n_m_b, n_ta_b;
  
  printf("Checking devices...\n");
  n_m_b = glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS);
  printf("Mouse with %d buttons.\n", n_m_b);
  h_ta = glutDeviceGet(GLUT_HAS_TABLET);
  n_ta_b = glutDeviceGet(GLUT_NUM_TABLET_BUTTONS);
  if (h_ta) {
    printf("Tablet with %d buttons detected.\n", n_ta_b);
  } else {
    printf("No tablet detected.\n");
  }
  printf("Done.\n");
  
  devs->has_tablet = h_ta;
  devs->num_mouse_buttons = n_m_b;
  devs->num_tablet_buttons = n_ta_b;
}
