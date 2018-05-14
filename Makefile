INCLUDES = -I /usr/include
CXXFLAGS = -Wall ${INCLUDES}
OBJS = bloxorz.o
PROG = bloxorz

SYS := $(shell uname -s)$

ifeq ($(SYS),Darwin)
	LIBS = -lGLEW -lglfw  -framework OpenGL -ldl
else
	LIBS            = -lGL -lglfw -lGLEW -ldl
endif

all:	$(PROG)

${PROG}:	$(OBJS)
	$(CXX) $(INCLUDES) -o $(PROG) $(OBJS) $(LIBS)

clean:;	$(RM) -f $(PROG) core *.o
