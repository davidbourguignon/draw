#include <assert.h>
#include "interface.h"

Cell::Cell() {}

Cell::Cell(const int id, const GLint x, const GLint y, char* name)
  : _id(id), _hit(false), _x(x), _y(y) {
  fflush(stdout);
  GLubyte* image = read_rgb_texture(name, &_width, &_height);
  assert(image != NULL);
  for (int i = 0; i < 3*_width*_height; i++) {
    _image.push_back(image[i]);
  }
  free(image);
  _xmax = _x + _width;
  _ymax = _y + _height;
}

bool Cell::isHit(int x, int y) {
  if ((x >= _x && x < _xmax) && (y >= _y && y < _ymax)) {
    _hit = true;
  }
  else {
    _hit = false;
  }
  return _hit;
}

int Cell::id() const {
  return _id;
}

void Cell::draw() const {
  glRasterPos2i(_x, _y);
  glDrawPixels(_width, _height, GL_RGB, GL_UNSIGNED_BYTE, &_image[0]);
}

void Cell::drawHit() const {
  glBegin(GL_LINE_STRIP);
  glVertex2f(_x - 1, _y);
  glVertex2f(_x - 1, _ymax);
  glVertex2f(_xmax, _ymax);
  glEnd();
}

/*****************************************************************************/

Toolbar::Toolbar()
  : _cells_list(0), _hit_id(-1), _hit_id_prev(-1), _hit_index(-1),
    _hit_index_prev(-1) {}

void Toolbar::addCell(const int id, const GLint x, const GLint y, char* name) {
  _cells.push_back(Cell(id, x, y, name));
}

void Toolbar::buildDisplayLists(const int window) {
  glutSetWindow(window);
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  _cells_list = glGenLists(1);
  if (_cells_list != 0) {
    glNewList(_cells_list, GL_COMPILE);
    for (int i = 0; i < _cells.size(); i++) {
      _cells[i].draw();
    }
    glEndList();
  }
  else {
    assert(false);
  }
  glPopClientAttrib();
}

void Toolbar::setHitID(const int hit_id) {
  _hit_id_prev = _hit_id;
  _hit_id = hit_id;
  for (int i = 0; i < _cells.size(); i++) {
    if (_cells[i].id() == hit_id) {
      _hit_index_prev = _hit_index;
      _hit_index = i;
      return;
    }
  }
}

bool Toolbar::findHit(int x, int y) {
  for (int i = 0; i < _cells.size(); i++) {
    if (_cells[i].isHit(x, y)) {
      _hit_id_prev = _hit_id;
      _hit_id = _cells[i].id();
      _hit_index_prev = _hit_index;
      _hit_index = i;
      return true;
    }
  }
  return false;
}

bool Toolbar::isHit() {
  if (_hit_index > -1) {
    return true;
  }
  else {
    return false;
  }
}

void Toolbar::cancelHit() {
  _hit_id = _hit_id_prev;
  _hit_index = _hit_index_prev;
}

int Toolbar::hitID() const {
  return _hit_id;
}

void Toolbar::draw() const {
  glCallList(_cells_list);
}

void Toolbar::drawHit() const {
  if (_hit_index > -1) {
    _cells[_hit_index].drawHit();
  }
}
