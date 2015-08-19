#
# draw.pro
# tmake project file
#
TEMPLATE	= app.t
CONFIG		= opengl debug
DEFINES		= HEAVY_MODELS #ALPHA_TEXTURE ANTIALIASING MULTITEXTURING TEST_TEXTURE
INCLUDEPATH = ./bezier ./aabb
LIBS		+= -lglut -lglui
#
SOURCES		=	draw.cc interface.cc \
				models_cc/greek_rev_house.cc \
				models_cc/babe_bw.cc \
				models_cc/heart0.cc \
				models_cc/woody.cc \
				models_cc/she_model.cc\
				drawing.cc texture.cc \
				stroke3D.cc stroke2D.cc input.cc opengl_utils.cc \
				texload.c widgets.c
TARGET      =	draw
