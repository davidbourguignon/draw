#
# draw.pro
# tmake project file
#
TEMPLATE	= app.t
CONFIG		= opengl debug
DEFINES		= TEST_STROKE2D
LIBS		+= -lglut
#
SOURCES     = input.cc stroke2D.cc drawing.C draw.C opengl_utils.cc
TARGET      = draw
