#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include <GL/glut.h>
#include "texload.h"

class Cell {
private:
  int _id;
  bool _hit;
  GLint _x, _y;       // OpenGL convention, not GLUT convention!
  GLint _xmax, _ymax; //
  GLsizei _width, _height;
  std::vector<GLubyte> _image;
  
public:
  Cell();
  Cell(const int id, const GLint x, const GLint y,
       char* name); // SGI .rgb image
  bool isHit(int x, int y);
  int id() const;
  void draw() const;
  void drawHit() const;
};

class Toolbar {
private:
  std::vector<Cell> _cells;
  GLuint _cells_list;
  int _hit_id, _hit_id_prev, _hit_index, _hit_index_prev;
  
public:
  Toolbar();
  void addCell(const int id, const GLint x, const GLint y, char* name);
  void buildDisplayLists(const int window);
  void setHitID(const int hit_id);
  bool findHit(int x, int y);
  bool isHit();
  void cancelHit();
  int hitID() const;
  void draw() const;
  void drawHit() const;
};

#endif // INTERFACE_H
