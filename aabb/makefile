#############################################################################
# Makefile for building aabb_test
# Generated by tmake at 11:46, 2004/09/06
#     Project: aabb_test
#    Template: app.t
#############################################################################

####### Compiler, tools and options

CC	=	gcc
CXX	=	g++
CFLAGS	=	-pipe -finline -Winline -g -DDEBUG
CXXFLAGS=	-pipe -finline -Winline -LANG:std -g -DDEBUG
INCPATH	=	-I/usr/X11R6/include
LINK	=	g++
LFLAGS	=	-LANG:std
LIBS	=	$(SUBLIBS) -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm
MOC	=	$(QTDIR)/bin/moc
UIC	=	$(QTDIR)/bin/uic

TAR	=	tar -cf
GZIP	=	gzip -9f

####### Files

HEADERS =	
SOURCES =	aabb_test.cc
OBJECTS =	aabb_test.o
INTERFACES =	
UICDECLS =	
UICIMPLS =	
SRCMOC	=	
OBJMOC	=	
DIST	=	
TARGET	=	aabb_test
INTERFACE_DECL_PATH = .

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules


all: $(TARGET)

$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

tmake: makefile

makefile: aabb_test.pro
	tmake aabb_test.pro -o makefile

dist:
	$(TAR) aabb_test.tar aabb_test.pro $(SOURCES) $(HEADERS) $(INTERFACES) $(DIST)
	$(GZIP) aabb_test.tar

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(UICIMPLS) $(UICDECLS) $(TARGET)
	-rm -f core.*

####### Sub-libraries


###### Combined headers


####### Compile

aabb_test.o: aabb_test.cc \
		aabb.h \
		vec2.h \
		numerics.h \
		vec3.h

