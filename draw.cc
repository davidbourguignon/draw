#include <GL/glui.h>
#include "input.h"
#include "drawing.h"
#include "interface.h"
#include "display_lists.h"

using namespace std;

/* Typedefs */
typedef GLdouble        real;
typedef Vec3<real>      vec3;
typedef Quat<real>      quat;
typedef Trackball<real> trackball;

/* Data */
// Windows
int board, tools, persp, command;
int board_width = 512;
int board_height = 512;
int tools_width = 110;
int tools_height = 256;
int border_size = 4;
float sub_window_ratio = 1.0/2.25;
int sub_size[2];
bool full_screen = false;
// Colors
GLfloat bg_color[4]    = {1.0, 1.0, 1.0, 1.0};
GLfloat st_color[4]    = {0.0, 0.0, 0.0, 1.0};
GLfloat selec_color[4] = {0.0, 0.0, 1.0, 1.0};
bool dark_st_color = true;
const GLfloat button_grey = 215.0/255.0;
// Display lists
GLuint axes_list, box_list, grid_list, persp_first_list, persp_second_list;
std::vector<GLuint> scene_lists;
int scene_id = 0;
bool draw_axes = false;
bool draw_scene = false;
bool draw_infos = false;
// Background texture and others
bool background_texture = false;
bool transparent_plane = false;
bool accumulation = false;
// Viewing
trackball tb_board;
trackball tb_board_copy;
GLdouble tb_board_matrix[4][4];
trackball tb_persp;
GLdouble tb_persp_matrix[4][4];
GLint command_viewport[4];
// Mouse
enum mousemode {DRAW, TRACK, EDIT};
mousemode mouse_mode = DRAW;
mousemode mouse_mode_prev = mouse_mode;
bool mouse_mode_locked = false;
// Commands
Toolbar tlbr_file;
Toolbar tlbr_mode;
Toolbar tlbr_tool;
enum commandmode {NO_HIT = -1, LOAD_FILE, SAVE_FILE, VOID_FILE,//tmp
		  GLOBAL_MODE, LOCAL_MODE, DELETE,
		  EDIT_STROKE, MOVE_STROKE, UNDO,
		  PENCIL, BRUSH, ERASER};
int tool_type = PENCIL;
//Picking
const GLsizei selection_buf_capacity = 4096; // Magic number!
GLuint selection_buf[selection_buf_capacity];
GLint selection_buf_size;
const GLsizei feedback_buf_capacity = 4096; // Magic number!
GLfloat feedback_buf_front[feedback_buf_capacity];
GLfloat feedback_buf_back[feedback_buf_capacity];
GLint feedback_buf_size_front, feedback_buf_size_back;
// GLUI
GLUI *file_io, *file_error, *save_warning;
enum gluiCallbackID {OK, CANCEL, SAVE};
GLUI_EditText *edittext;
GLUI_String file_name = "";
GLUI_String file_name_old = "";
GLUI_String input_file_name = "";
// File
enum filemode {STOP, PLAY, STEP, RECORD, RECORD_AND_QUIT,
	       PLAY_INPUT, RECORD_INPUT};
filemode file_mode = STOP;
bool rec_input = false;
bool drawing_saved = true;
// Others
Input I;
Drawing D;

/* Functions definition */
void setColors() {
  glutSetWindow(board);
  glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
  glutSetWindow(tools);
  glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
  glutSetWindow(persp);
  glClearColor(button_grey, button_grey, button_grey, 1.0);
  glutSetWindow(command);
  glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
  
  D.setColor(bg_color,    Drawing::BACKGROUND_COLOR);
  D.setColor(st_color,    Drawing::STROKE_COLOR);
  D.setColor(selec_color, Drawing::SELECTED_STROKE_COLOR);
  I.setPointColor(st_color);
}

void setSubSize(const GLint width, const GLint height) {
  sub_size[0] = static_cast<int>(width - 2*border_size);
  sub_size[1] = static_cast<int>((height - 3*border_size)*sub_window_ratio);
}

void checkDevices() {
  cout << "Checking devices..." << endl;
  int has_keyboard = glutDeviceGet(GLUT_HAS_KEYBOARD);
  if (has_keyboard) {
    cout << "Keyboard detected." << endl;
  }
  else {
    cout << "No keyboard detected." << endl;
  }
  int has_mouse = glutDeviceGet(GLUT_HAS_MOUSE);
  int num_mouse_buttons = glutDeviceGet(GLUT_NUM_MOUSE_BUTTONS);
  if (has_mouse) {
    cout << "Mouse with " << num_mouse_buttons << " buttons detected." << endl;
  }
  else {
    cout << "No mouse detected." << endl;
  }
  int has_tablet = glutDeviceGet(GLUT_HAS_TABLET);
  int num_tablet_buttons = glutDeviceGet(GLUT_NUM_TABLET_BUTTONS);
  if (has_tablet) {
    cout << "Tablet with " << num_tablet_buttons << " buttons detected."
	 << endl;
  }
  else {
    cout << "No tablet detected." << endl;
  }
  cout << endl;
}

void initBoard() {
  glutSetWindow(board);
  
  /* Display lists */
  // Axes
  axes_list = glGenLists(1);
  if (axes_list) {
    glNewList(axes_list, GL_COMPILE);
    axes();
    glEndList();
  }
  else {
    assert(false);
  }
  // Teapot
  GLuint scene_list;
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    teapot(1.0);
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
  
#if HEAVY_MODELS
  // Greek Revival House
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    greekRevivalHouse();
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
  // Babe BW
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    babeBW();
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
  // Heart
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    heart0();
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
  // Woody
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    woody();
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
  // She model
  scene_list = glGenLists(1);
  if (scene_list) {
    glNewList(scene_list, GL_COMPILE);
    sheModel();
    glEndList();
  }
  else {
    assert(false);
  }
  scene_lists.push_back(scene_list);
#endif
  
  /* Textures */
  std::vector<GLsizei> dim_1D(1, 128);
  std::vector<GLsizei> dim_2D(2, 128);
  D.addTexture(Texture("tex/fabric.rgb"), Drawing::BACKGROUND);
  D.addTexture(Texture(Gauss(0.0, 0.5), 2, dim_2D,
	               Texture::CIRCULAR_GAUSSIAN_FILTER), Drawing::OCCLUDER);
  D.addTexture(Texture(Gauss(0.0, 50.0), 2, dim_2D), Drawing::PROBA_SURFACE);
#if TEST_TEXTURE
  //D.addTexture(Texture("tex/pencil.rgb"), Drawing::PROBA_SURFACE);
  D.addTexture(Texture("tex/brush.rgb"), Drawing::STROKE);
#else // TEST_TEXTURE
#if ALPHA_TEXTURE
  //D.addTexture(Texture("tex/pencil_invert_alpha.rgb",
  //	         Texture::SGI_ALPHA, Texture::ALPHA),
  //             Drawing::PROBA_SURFACE);
  D.addTexture(Texture("tex/brush_invert_alpha.rgb",
		       Texture::SGI_ALPHA, Texture::ALPHA), Drawing::STROKE);
#else // ALPHA_TEXTURE
  //D.addTexture(Texture("tex/pencil_invert_rgba.rgb",
  //	         Texture::SGI_RGBA,  Texture::RGBA),
  //             Drawing::PROBA_SURFACE);
  D.addTexture(Texture("tex/brush_invert_rgba.rgb",
		       Texture::SGI_RGBA,  Texture::RGBA), Drawing::STROKE);
#endif // ALPHA_TEXTURE
#endif // TEST_TEXTURE
}

void initPersp() {
  glutSetWindow(persp);
  
  // Settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
#if ANTIALIASING
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
  
  // Display lists
  box_list = glGenLists(1);
  if (box_list) {
    glNewList(box_list, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0.0, 0.0, -11.0);//
    glRotatef(5.0, 1.0, 0.0, 0.0);//
    glTranslatef(3.0, 0.0, 0.0);  //
    glTranslatef(0.0, 1.5, 0.0);  //
    box(3.0, 3.0, 3.0);
    glPopMatrix();
    glEndList();
  }
  else {
    assert(false);
  }
  grid_list = glGenLists(1);
  if (grid_list) {
    glNewList(grid_list, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0.0, 0.0, -11.0);//
    glRotatef(5.0, 1.0, 0.0, 0.0);//
    grid(20.0, 20.0, 20, 20);
    glTranslatef(-3.0, 0.0, 0.0); //
    glTranslatef(0.0, 1.5, 0.0);  //
    box(3.0, 3.0, 3.0);
    glPopMatrix();
    glEndList();
  }
  else {
    assert(false);
  }
  persp_first_list = glGenLists(1);
  if (persp_first_list) {
    glNewList(persp_first_list, GL_COMPILE);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0, 0.0, 0.0);
    glCallList(grid_list);
    glEndList();
  }
  else {
    assert(false);
  }
  persp_second_list = glGenLists(1);
  if (persp_second_list) {
    glNewList(persp_second_list, GL_COMPILE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(button_grey, button_grey, button_grey);
    glCallList(grid_list);
    glEndList();
  }
  else {
    assert(false);
  }
}

void initCommand() {
  const GLint p[4][3][2] = {{{2, 104}, {36, 104}, {70, 104}},
                            {{2,  70}, {36,  70}, {70,  70}},
                            {{2,  36}, {36,  36}, {70,  36}},
                            {{2,   2}, {36,   2}, {70,   2}}};
  
  tlbr_file.addCell(LOAD_FILE,p[0][0][0], p[0][0][1], "icons/load.rgb");
  tlbr_file.addCell(SAVE_FILE,p[0][1][0], p[0][1][1], "icons/save.rgb");
  tlbr_file.addCell(VOID_FILE,
		              p[0][2][0], p[0][2][1], "icons/void.rgb");
  tlbr_mode.addCell(GLOBAL_MODE,
		              p[1][0][0], p[1][0][1], "icons/global.rgb");
  tlbr_mode.addCell(LOCAL_MODE,
		              p[1][1][0], p[1][1][1], "icons/local.rgb");
  tlbr_file.addCell(DELETE,   p[1][2][0], p[1][2][1], "icons/delete.rgb");
  
  tlbr_tool.addCell(EDIT_STROKE,
		              p[2][0][0], p[2][0][1], "icons/pointer.rgb");
  tlbr_tool.addCell(MOVE_STROKE,
		              p[2][1][0], p[2][1][1], "icons/move.rgb");
  tlbr_file.addCell(UNDO,     p[2][2][0], p[2][2][1], "icons/undo.rgb");
  tlbr_tool.addCell(PENCIL,   p[3][0][0], p[3][0][1], "icons/pencil.rgb");
  tlbr_tool.addCell(BRUSH,    p[3][1][0], p[3][1][1], "icons/brush.rgb");
  tlbr_tool.addCell(ERASER,   p[3][2][0], p[3][2][1], "icons/eraser.rgb");
  
  tlbr_file.buildDisplayLists(command);
  tlbr_mode.buildDisplayLists(command);
  tlbr_tool.buildDisplayLists(command);
  
  tlbr_mode.setHitID(GLOBAL_MODE);
  tlbr_tool.setHitID(tool_type);
}

void init() {
  // Check devices
  checkDevices();
  
  // Colors
  setColors();
  
  // Windows
  initBoard();
  initPersp();
  initCommand();
  
  // Trackballs
  quat qInit1 = quat(vec3(1.0, 0.0, 0.0), -M_PI/32.0);
  quat qInit2 = quat(vec3(0.0, 1.0, 0.0),  M_PI/16.0);
  quat qInit  = qInit1*qInit2;
  vec3 vInit  = vec3(0.0, 0.0, -2.05);
  tb_board    = trackball(qInit, vInit);
  
  // Other inits
  I.window = board;
  I.point_size = 1.0;
  D.point_size = 1.0;
  D.line_width = 5.0;
}

void redisplay() {
  glutPostWindowRedisplay(board);
  glutPostWindowRedisplay(tools);
  glutPostWindowRedisplay(persp);
  glutPostWindowRedisplay(command);
}

void chooseModel() {
  cout << endl;
  cout << "M o d e l s" << endl;
  cout << "0\tTeapot (default)" << endl;
  cout << "1\tGreek Revival House" << endl;
  cout << "2\tBabe B&W" << endl;
  cout << "3\tHeart" << endl;
  cout << "4\tWoody" << endl;
  cout << "5\tShe model" << endl;
  const int nmax = 6;
  cout << nmax << "\tQuit" << endl;
  cout << endl;
  cout << "Please enter your choice: ";
  bool wrong_input = true;
  while (wrong_input) {
    int n;
    cin >> n;
    if (!cin) {
      cin.clear();
      char dump[256];
      cin >> dump;
      cout << "Error: Invalid value!" << endl;
      cout << "Please enter your choice: ";
    }
    else {
      if (n < 0 || n > nmax) {
        cout << "Error: Wrong choice!" << endl;
        cout << "Please enter your choice: ";
      }
      else {
        wrong_input = false;
        if (n != nmax) {
	  scene_id = n;
	  cout << "Thank you!" << endl;
        }
        else {
	  cout << "Goodbye!" << endl;
	}
        cout << endl;
      }
    }
  }
}

/* Callbacks */
// Drawing board window
void boardReshape(int width, int height) {
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  glGetIntegerv(GL_VIEWPORT, I.viewport);
  tb_board.reshape(width, height);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  I.fovy = 60.0;
  I.aspect = static_cast<GLdouble>(width)/static_cast<GLdouble>(height);
  I.near = 1.0; I.far = 10.0;
  gluPerspective(I.fovy, I.aspect, I.near, I.far);
  glGetDoublev(GL_PROJECTION_MATRIX, I.proj_matrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  tb_board.writeOpenGLTransfMatrix(tb_board_matrix);
  glPushMatrix();
  glMultMatrixd(&tb_board_matrix[0][0]);
  glGetDoublev(GL_MODELVIEW_MATRIX, I.mv_matrix);
  I.setViewVector();
  I.setGlobalPlane();
  D.setBackgroundVertices(I);
  glPopMatrix();
}

void boardDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (tb_board.isMoved()) {
    tb_board.writeOpenGLTransfMatrix(tb_board_matrix);
  }
  glPushMatrix();
  glMultMatrixd(&tb_board_matrix[0][0]);
  if (tb_board.isMoved()) {
    glGetDoublev(GL_MODELVIEW_MATRIX, I.mv_matrix);
    I.setViewVector();
    I.setGlobalPlane();
    D.setBackgroundVertices(I);
  }
  if (background_texture) {
    D.paintBackground();
  }
  if (draw_scene) {
    glCallList(scene_lists[scene_id]);
  }
  if (draw_axes) {
    glPushAttrib(GL_CURRENT_BIT);
    glColor4fv(st_color);
    glCallList(axes_list);
    glPopAttrib();
  }
  if (transparent_plane) {
    D.paintTransparentPlane();
  }
  I.draw(); // TODO: Find a condition to avoid drawing empty inputs
  if (draw_infos) {
    D.drawInformations(I);
  }
  else {
    D.draw(I);
  }
  glPopMatrix();
  
  glutSwapBuffers();
}

void boardSelection(int x, int y) {
  glSelectBuffer(selection_buf_capacity, selection_buf);
  static_cast<GLvoid>(glRenderMode(GL_SELECT));
  glInitNames();
  glPushName(Input::DEFAULT_NAME);
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLdouble winx = static_cast<GLdouble>(x);
  GLdouble winy = static_cast<GLdouble>(I.viewport[3] - 1 - y);
  gluPickMatrix(winx, winy, 1.0, 1.0, I.viewport);
  gluPerspective(I.fovy, I.aspect, I.near, I.far);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixd(&tb_board_matrix[0][0]);
  if (draw_scene) {
    glCallList(scene_lists[scene_id]);
  }
  D.drawSelection();
  glPopMatrix();
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
  glMatrixMode(GL_MODELVIEW);
  
  selection_buf_size = glRenderMode(GL_RENDER);
  if (selection_buf_size == -1) {
    assert(false);
  }
  glFlush();
}

void boardFeedback(GLint& feedback_buf_size, GLfloat* feedback_buf,
		   int x, int y) {
  glFeedbackBuffer(feedback_buf_capacity, GL_3D, feedback_buf);
  static_cast<GLvoid>(glRenderMode(GL_FEEDBACK));
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLdouble winx = static_cast<GLdouble>(x);
  GLdouble winy = static_cast<GLdouble>(I.viewport[3] - 1 - y);
  gluPickMatrix(winx, winy, 1.0, 1.0, I.viewport);
  gluPerspective(I.fovy, I.aspect, I.near, I.far);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixd(&tb_board_matrix[0][0]);
  if (draw_scene) {
    glPassThrough(I.object_tokens[Input::SCENE_INDEX]);
    glCallList(scene_lists[scene_id]);
  }
  D.drawFeedback(I);
  glPopMatrix();
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
  glMatrixMode(GL_MODELVIEW);
  
  feedback_buf_size = glRenderMode(GL_RENDER);
  if (feedback_buf_size == -1) {
    assert(false);
  }
  glFlush();
}

void boardKeyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'a':
    if (draw_axes) {
      draw_axes = false;
    }
    else {
      draw_axes = true;
    }
    break;
  case 'b':
    if (background_texture) {
      background_texture = false;
    }
    else {
      background_texture = true;
    }
    break;
  case 'c':
    if (dark_st_color) {
      dark_st_color = false;
      bg_color[0] = 0.0; bg_color[1] = 0.0;
      bg_color[2] = 0.0; bg_color[3] = 1.0;
      st_color[0] = 1.0; st_color[1] = 1.0;
      st_color[2] = 1.0; st_color[3] = 1.0;
      setColors();
    }
    else {
      dark_st_color = true;
      bg_color[0] = 1.0; bg_color[1] = 1.0;
      bg_color[2] = 1.0; bg_color[3] = 1.0;
      st_color[0] = 0.0; st_color[1] = 0.0;
      st_color[2] = 0.0; st_color[3] = 1.0;
      setColors();
    }
    break;
  case 'f':
    if (full_screen) {
      full_screen = false;
      glutSetWindow(board);
      glutPositionWindow(200, 200); // Magic number!
      glutReshapeWindow(board_width, board_height);
      glutSetWindow(tools);
      glutPositionWindow(50, 200); // Magic number!
    }
    else {
      full_screen = true;
      glutSetWindow(board);
      glutFullScreen();
      glutSetWindow(tools);
      glutPositionWindow(50, 50); // Magic number!
    }
    break;
  case 'g':
    if (draw_infos) {
      draw_infos = false;
    }
    else {
      draw_infos = true;
    }
    break;
  case 'h':
    printf("\n");
    printf("H e l p !\n");
    printf("a\tdraw Axes switch\n");
    printf("b\tBackground texture switch\n");
    printf("c\tColor switch\n");
    printf("f\tFull screen switch\n");
    printf("g\tdebuG infos switch\n");
    printf("h\tdisplay Help\n");
    printf("i\treInitialize trackball\n");
    printf("l\tLoad data file in step mode\n");
    printf("m\tModel choice\n");
    printf("o\tplay One step\n");
    printf("p\tPlay input data file\n");
    printf("q\tQuit\n");
    printf("r\tRecord input data file switch\n");
    printf("s\tdraw Scene model switch\n");
    printf("t\tsemi-Transparent drawing plane switch\n");
    printf("u\taccUmulation switch (deprecated)\n");
    printf("v\treVerse stroke\n");
    break;
  case 'i':
    tb_board.reinitializeTransf();
    break;
  case 'l':
    file_mode = STEP;
    file_name_old = file_name;
    file_io->show();
    break;
  case 'm':
    chooseModel();
    break;
  case 'o':
    if (!D.readOneByOne(file_name, board)) {
      file_error->show();
    }
    break;
  case 'p':
    file_mode = PLAY_INPUT;
    file_name_old = file_name;
    file_io->show();
    break;
  case 'q':
  case 27:
    if (drawing_saved) {
      exit(EXIT_SUCCESS);
    }
    else {
      save_warning->show();
    }
    break;
  case 'r':
    if (rec_input) {
      rec_input = false;
    }
    else {
      file_mode = RECORD_INPUT;
      file_name_old = file_name;
      file_io->show();
    }
    break;
  case 's':
    if (draw_scene) {
      draw_scene = false;
    }
    else {
      draw_scene = true;
    }
    break;
  case 't':
    if (transparent_plane) {
      transparent_plane = false;
      tb_board = tb_board_copy;
      tb_board.change();
    }
    else {
      transparent_plane = true;
      D.setTransparentPlaneVertices(I);
      tb_board_copy = tb_board;
    }
    break;
  case 'u':
    if (accumulation) {
      accumulation = false;
      D.setAccumulationMode(false);
    }
    else {
      accumulation = true;
      D.setAccumulationMode();
    }
    break;
  case 'v':
    D.reverseStroke(I);
    break;
  default:
    fprintf(stderr, "Error: Unknown option !\n");
    break;
  }
  redisplay();
}

void boardMouse(int button, int state, int x, int y) {
  switch (button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
      if (mouse_mode_locked) {
	mouse_mode = mouse_mode_prev;
	mouse_mode_locked = false;
      }
      if (mouse_mode == EDIT) {
	if (tool_type == EDIT_STROKE) {
	  boardSelection(x, y);
	  if (I.selectStroke(selection_buf_size, selection_buf)) {
	    D.markStroke(I);
	  }
        }
        else if (tool_type == MOVE_STROKE) {
	  D.startMovingStroke(x, y);
        }
        else {
	  assert(false);
        }
      }
      else if (mouse_mode == DRAW) {
	D.unmarkStroke();
      }
      else {
	assert(false);
      }
      tb_board.change(false);
    }
    else {
      if (mouse_mode == DRAW) {
	int winx_front = static_cast<int>(I.positions.front().pos.x());
        int winy_front = static_cast<int>(I.positions.front().pos.y());
        int winx_back  = static_cast<int>(I.positions.back().pos.x());
        int winy_back  = static_cast<int>(I.positions.back().pos.y());
        boardFeedback(feedback_buf_size_front, feedback_buf_front,
		      winx_front, winy_front);
	boardFeedback(feedback_buf_size_back, feedback_buf_back,
		      winx_back, winy_back);
	I.setPlanes(feedback_buf_size_front, feedback_buf_front,
		    feedback_buf_size_back, feedback_buf_back);
	
	if (tool_type == PENCIL) {
	  D.addStroke(Stroke3D(I, Stroke2D(I), Stroke3D::LINE));
        }
        else if (tool_type == BRUSH) {
	  D.addStroke(Stroke3D(I, Stroke2D(I), Stroke3D::TEXTURED_POLYGON));
        }
        else if (tool_type == ERASER) {
	  D.addStroke(Stroke3D(I, Stroke2D(I), Stroke3D::OCCLUSION));
        }
	else {
	  cerr << "MOUSE MODE DRAW" << endl;//tmp
	  cerr << "TOOL TYPE " << tool_type << endl;//tmp
	  assert(false);
        }
      }
      else if (mouse_mode == EDIT) {
	if (tool_type == EDIT_STROKE) {
        }
        else if (tool_type == MOVE_STROKE) {
	  D.stopMovingStroke(x, y, I);
        }
        else {
	  assert(false);
        }
      }
      else {
	cerr << "MOUSE MODE " << mouse_mode << endl;//tmp
	assert(false);
      }
      
      drawing_saved = false;
      if (rec_input) {
	I.write(input_file_name);
      }
      I.clear();
    }
    break;
  case GLUT_MIDDLE_BUTTON:
    if (state == GLUT_DOWN) {
      if (!mouse_mode_locked) {
	mouse_mode_prev = mouse_mode;
	mouse_mode_locked = true;
      }
      mouse_mode = TRACK;
      tb_board.startTranslationZ(x, y);
    }
    else {
      tb_board.stopTranslationZ();
    }
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN) {
      if (!mouse_mode_locked) {
	mouse_mode_prev = mouse_mode;
	mouse_mode_locked = true;
      }
      mouse_mode = TRACK;
      tb_board.startRotation(x, y);
    }
    else {
      tb_board.stopRotation();
    }
    break;
  }
  glutPostRedisplay();
}

void boardMotion(int x, int y) {
  if (mouse_mode == DRAW) {
    I.addPoint(x, y);
  }
  else if (mouse_mode == TRACK) {
    tb_board.move(x, y);
  }
  else if (mouse_mode == EDIT) {
    if (tool_type == EDIT_STROKE) {
    }
    else if (tool_type == MOVE_STROKE) {
    }
    else {
      cerr << "MOUSE MODE EDIT" << endl;//tmp
      cerr << "TOOL TYPE " << tool_type << endl;//tmp
      assert(false);
    }
  }
  else {
    assert(false);
  }
  glutPostRedisplay();
}

// Tools window
void toolsReshape(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, width, 0.0, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  setSubSize(width, height);
  glutSetWindow(persp);
  glutPositionWindow(border_size, border_size);
  glutReshapeWindow(sub_size[0], sub_size[1]);
  glutSetWindow(command);
  glutPositionWindow(border_size, sub_size[1] + 2*border_size);
  glutReshapeWindow(sub_size[0], height - sub_size[1] - 3*border_size);
}

void toolsDisplay() {
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();
}

// Perspective window
void perspReshape(int width, int height) {
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLdouble aspect = static_cast<GLdouble>(width)/static_cast<GLdouble>(height);
  gluPerspective(60.0, aspect, 1.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  tb_persp.reshape(width, height);
  tb_persp.writeOpenGLTransfMatrix(tb_persp_matrix);
}

void perspDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (tb_persp.isMoved()) {
    tb_persp.writeOpenGLTransfMatrix(tb_persp_matrix);
    GLdouble offset = -0.1*tb_persp_matrix[3][2]; // Magic number!
    I.setGlobalPlaneOffset(offset);
  }
  glCallList(persp_first_list);
  glPushMatrix();
  glMultMatrixd(&tb_persp_matrix[0][0]);
  glCallList(box_list);
  glPopMatrix();
  glCallList(persp_second_list);
  glPushMatrix();
  glMultMatrixd(&tb_persp_matrix[0][0]);
  glCallList(box_list);
  glPopMatrix();
  
  glutSwapBuffers();
}

void perspKeyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'h':
    printf("\n");
    printf("H e l p !\n");
    printf("i\treInitialize trackball\n");
    printf("q\tQuit\n");
    break;
  case 'i':
    tb_persp.reinitializeTransf();
    break;
  case 'q':
  case 27:
    if (drawing_saved) {
      exit(EXIT_SUCCESS);
    }
    else {
      save_warning->show();
    }
    break;
  default:
    fprintf(stderr, "Error: Unknown option !\n");
    break;
  }
  glutPostRedisplay();
}

void perspMouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    tb_persp.startTranslationZ(x, y);
  }
  else if (state == GLUT_UP) {
    tb_persp.stopTranslationZ();
  }
}

void perspMotion(int x, int y) {
  tb_persp.move(x, y);
  glutPostRedisplay();
}

// Command window
void commandReshape(int width, int height) {
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, static_cast<GLdouble>(width),
	     0.0, static_cast<GLdouble>(height));
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glGetIntegerv(GL_VIEWPORT, command_viewport);
}

void commandDisplay() {
  glClear(GL_COLOR_BUFFER_BIT);
  glPushAttrib(GL_CURRENT_BIT);
  glColor4fv(st_color);
  tlbr_file.draw();
  tlbr_mode.draw();
  tlbr_tool.draw();
  tlbr_file.drawHit();
  tlbr_mode.drawHit();
  tlbr_tool.drawHit();
  glPopAttrib();
  glutSwapBuffers();
}

void commandMouse(int button, int state, int x, int y) {
  int y_OpenGL = command_viewport[3] - 1 - y;
  int id;
  if (state == GLUT_DOWN) {
    if (tlbr_file.findHit(x, y_OpenGL)) {
      id = tlbr_file.hitID();
    }
    else if (tlbr_mode.findHit(x, y_OpenGL)) {
      id = tlbr_mode.hitID();
    }
    else if (tlbr_tool.findHit(x, y_OpenGL)) {
      id = tlbr_tool.hitID();
    }
    else {
      id = NO_HIT;
    }
    
    /* Parse */
    if (id == NO_HIT) {
    }
    else if (id == LOAD_FILE) {
      file_mode = PLAY;
      file_name_old = file_name;
      file_io->show();
    }
    else if (id == SAVE_FILE) {
      file_mode = RECORD;
      file_name_old = file_name;
      file_io->show();
    }
    else if (id == VOID_FILE) {//tmp
      tlbr_file.cancelHit();
    }
    else if (id == GLOBAL_MODE) {
      I.setLocalPlaneMode(false);
    }
    else if (id == LOCAL_MODE) {
      I.setLocalPlaneMode();
    }
    else if (id == DELETE) {
      D.clearStrokes(I);
      drawing_saved = true;
    }
    else if (id == EDIT_STROKE || id == MOVE_STROKE) {
      mouse_mode = EDIT;
      mouse_mode_locked = false;
      glutSetWindow(board);
      if (id == EDIT_STROKE) {
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
      }
      else {
	glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
      }
      tool_type = id;
    }
    else if (id == UNDO) {
      D.removeStroke(I);
      drawing_saved = false;
    }
    else if (id == PENCIL || id == BRUSH || id == ERASER) {
      mouse_mode = DRAW;
      mouse_mode_locked = false;
      glutSetWindow(board);
      glutSetCursor(GLUT_CURSOR_CROSSHAIR);
      tool_type = id;
    }
    else {
      assert(false);
    }
  }
  else if (state == GLUT_UP) {
    if (tlbr_file.isHit()) {
      id = tlbr_file.hitID();
    }
    else if (tlbr_mode.isHit()) {
      id = tlbr_mode.hitID();
    }
    else if (tlbr_tool.isHit()) {
      id = tlbr_tool.hitID();
    }
    else {
      id = NO_HIT;
    }
    
    /* Parse */
    if (id == LOAD_FILE || id == SAVE_FILE || id == DELETE || id == UNDO) {
      tlbr_file.cancelHit();
    }
    else {
      // Do nothing
    }
  }
  redisplay();
}

void gluiFileIOCallback(int id) {
  switch (id) {
  case OK:
    switch (file_mode) {
    case STOP:
      break;
    case PLAY:
      if (!D.read(file_name, board)) {
	file_error->show();
      }
      else {
	file_io->hide();
      }
      break;
    case STEP:
      if (!D.readOneByOne(file_name, board)) {
	file_error->show();
      }
      else {
	file_io->hide();
      }
      break;
    case RECORD:
      if (!D.write(file_name)) {
	file_error->show();
      }
      else {
	drawing_saved = true;
	file_io->hide();
      }
      break;
    case RECORD_AND_QUIT:
      if (!D.write(file_name)) {
	file_error->show();
      }
      else {
	drawing_saved = true;
	file_io->hide();
	exit(EXIT_SUCCESS);
      }
      break;
    case PLAY_INPUT:
      if (!I.read(file_name)) {
	file_error->show();
      }
      else {
	input_file_name = file_name;
	D.addStroke(Stroke3D(I, Stroke2D(I)));
	drawing_saved = false;
	I.clear();
	file_io->hide();
      }
      break;
    case RECORD_INPUT:
      if (!I.write(file_name)) {
	file_error->show();
      }
      else {
	input_file_name = file_name;
	rec_input = true;
	file_io->hide();
      }
      break;
    default:
      assert(false);
      break;
    }
    break;
  case CANCEL:
    file_name = file_name_old;
    edittext->set_text(file_name_old);
    file_io->hide();
    break;
  default:
    assert(false);
    break;
  }
  redisplay();
}

void gluiFileErrorCallback(int id) {
  switch (id) {
  case OK:
    file_error->hide();
    break;
  default:
    assert(false);
    break;
  }
}

void gluiSaveWarningCallback(int id) {
  switch (id) {
  case OK:
    save_warning->hide();
    exit(EXIT_SUCCESS);
    break;
  case SAVE:
    save_warning->hide();
    file_mode = RECORD_AND_QUIT;
    file_name_old = file_name;
    file_io->show();
    break;
  case CANCEL:
    save_warning->hide();
    break;
  default:
    assert(false);
    break;
  }
}

void buildGLUIFileIO() {
  file_io = GLUI_Master.create_glui("F i l e   I / O");
  
  edittext = file_io->add_edittext("File Name: ", GLUI_EDITTEXT_TEXT,
				   file_name);
  edittext->set_w(200);
  edittext->set_alignment(GLUI_ALIGN_CENTER);
  GLUI_Panel* panel = file_io->add_panel("", GLUI_PANEL_NONE);
  file_io->add_button_to_panel(panel, "OK", OK,
			       (GLUI_Update_CB) gluiFileIOCallback);
  file_io->add_column_to_panel(panel, false);
  file_io->add_button_to_panel(panel, "Cancel", CANCEL,
			       (GLUI_Update_CB) gluiFileIOCallback);
  
  file_io->set_main_gfx_window(board);
  file_io->hide();
}

void buildGLUIFileError() {
  file_error = GLUI_Master.create_glui("E r r o r");
  
  GLUI_Panel* panel1 = file_error->add_panel("", GLUI_PANEL_NONE);
  file_error->add_statictext_to_panel(panel1, "Error: Can not open file!");
  GLUI_Panel* panel2 = file_error->add_panel("", GLUI_PANEL_NONE);
  file_error->add_button_to_panel(panel2, "OK", OK,
				  (GLUI_Update_CB) gluiFileErrorCallback);
  
  file_error->set_main_gfx_window(board);
  file_error->hide();
}

void buildGLUISaveWarning() {
  save_warning = GLUI_Master.create_glui("W a r n i n g");
  
  GLUI_Panel* panel1 = save_warning->add_panel("", GLUI_PANEL_NONE);
  save_warning->add_statictext_to_panel(panel1,
					"Drawing not saved. Quit anyway?");
  GLUI_Panel* panel2 = save_warning->add_panel("", GLUI_PANEL_NONE);
  save_warning->add_button_to_panel(panel2, "OK", OK,
				    (GLUI_Update_CB) gluiSaveWarningCallback);
  save_warning->add_column_to_panel(panel2, false);
  save_warning->add_button_to_panel(panel2, "Save", SAVE,
				    (GLUI_Update_CB) gluiSaveWarningCallback);
  save_warning->add_column_to_panel(panel2, false);
  save_warning->add_button_to_panel(panel2, "Cancel", CANCEL,
				    (GLUI_Update_CB) gluiSaveWarningCallback);
  
  save_warning->set_main_gfx_window(board);
  save_warning->hide();
}

/* Main program */
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  
  // Drawing board window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA |
		      GLUT_STENCIL | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowSize(board_width, board_height);
  board = glutCreateWindow("T h e   D r a w i n g   B o a r d");
  glutReshapeFunc(boardReshape);
  glutDisplayFunc(boardDisplay);
  glutKeyboardFunc(boardKeyboard);
  glutMouseFunc(boardMouse);
  glutMotionFunc(boardMotion);
  glutSetCursor(GLUT_CURSOR_CROSSHAIR);
  
  // Tools window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(tools_width, tools_height);
  tools = glutCreateWindow(" ");
  glutReshapeFunc(toolsReshape);
  glutDisplayFunc(toolsDisplay);
  glutKeyboardFunc(boardKeyboard);
  
  setSubSize(tools_width, tools_height);
  
  // Perspective window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
  persp = glutCreateSubWindow(tools, border_size, border_size,
			      sub_size[0], sub_size[1]);
  glutReshapeFunc(perspReshape);
  glutDisplayFunc(perspDisplay);
  glutKeyboardFunc(perspKeyboard);
  glutMouseFunc(perspMouse);
  glutMotionFunc(perspMotion);
  
  // Command window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
  command = glutCreateSubWindow(tools, border_size,
				sub_size[1] + 2*border_size, sub_size[0],
				tools_height - sub_size[1] - 3*border_size);
  glutReshapeFunc(commandReshape);
  glutDisplayFunc(commandDisplay);
  glutKeyboardFunc(boardKeyboard);
  glutMouseFunc(commandMouse);
  
  // GLUI
  GLUI_Master.set_glutIdleFunc(0);
  buildGLUIFileIO();
  buildGLUIFileError();
  buildGLUISaveWarning();
  
  // Init
  init();
  glutMainLoop();
  return 0;
}

/* TODO:
   . Missing features in the interface:
       multiselection,
       copy, cut & paste,
       translation with trackball.
   . Record camera position (viewpoint).
   . Type (or write) text on billboards always facing camera.
   . Debug pencil tool w/o bacground texture on onyx graphics boards.
   . Use tablet stylus (modify GLUT code).
*/
