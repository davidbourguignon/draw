Drawing for Illustration and Annotation in 3D.
David Bourguignon, Marie-Paule Cani and George Drettakis.
Computer Graphics Forum, 20(3):114-122, 2001.
---------------------------------------------------------

License
-------
Copyright (c) 2001 iMAGIS-GRAVIR / IMAG-INRIA

Requirements
------------
glui-2.2 (http://glui.sourceforge.net/)
glut-3.7 (http://freeglut.sourceforge.net/)
OpenGL-1.2 (http://www.opengl.org/)
tmake-2.12 (http://tmake.sourceforge.net/), for generating makefiles

Output
------
DR format (our own plain recording of the drawing)

Compiling
---------
With gcc-3.3.1 under linux-2.4, setup the makefile and type 'make'. There
are test programs in subdirectories called aabb, bezier and draw2D.

This program does not currently compile with msvc-13.10.3052 under mswinxp.
This is mostly due to the heavy use of arcane template features. However,
you can try compiling this program with mingw (http://mingw.sourceforge.net/),
an excellent mswin port of gcc that produces binaries in native mswin format.

Manual
------
There is a command-line help: you can reach it by pressing the "h" key. It
describes the keyboard commands.

Description of the icons in the toolbar (from left to right and top to
bottom):
- Two cubes in perspective: by clicking on the one on the right, you can
change the semi-transparent drawing plane position.
- Folder with outside-going arrow: Open previous drawing (in DR format).
- Folder with inside-going arrow: Save current drawing (in DR format).
- House: Switch to global drawing mode (strokes projected on the drawing
plane, ignore other strokes and objects of the scene).
- House with a pointing arrow: Switch to local drawing mode (strokes depth
is obtained using other strokes and objects of the scene).
- Cross mark: Delete all strokes of the current drawing.
- Simple pointer: Select a stroke by picking (this stroke is then
highlighted in blue).
- Cross pointer: Move a previously selected stroke in a plane parallel to
the current view plane.
- U-turn arrow: Undo last stroke or delete currently selected stroke.
- Pencil: Draw line stroke with foreground color (default: black).
- Brush: Draw silhouette stroke with foreground color (default: black).
- Eraser: Draw silhouette stroke with background color (default: white).

Enjoy! And please send me your complaints/comments/congrats. Thanks!

http://www.davidbourguignon.net/
