#
# bezier_test.pro
# tmake project file
#
TEMPLATE	= app.t
CONFIG		= opengl debug
DEFINES		= FLOAT
INCLUDEPATH = ./
LIBS		+= -lglut -lglui
#
SOURCES     = bezier_test.cc widgets.c
TARGET      = bezier_test
