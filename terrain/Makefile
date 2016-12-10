CC = g++
CFLAGS = -fno-builtin -Wall -Wno-deprecated -O3 -c
LDFLAGS = -O3 -o
HEADERPATH = -I /usr/X11R6/include -I /usr/include
LIBPATH = -L /usr/local/lib -L /usr/X11R6/lib -L /usr/lib
LIBS = -lm -lglut -lGL -lGLU -lXi -lXmu -lX11
OBJ = data_types.o imageReader.o terraview.o

TARGETS=terraview

all: $(TARGETS)

terraview: $(OBJ)
	$(CC) $(LDFLAGS) $@ $(OBJ) $(LIBPATH) $(LIBS)

data_types.o: data_types.cpp data_types.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

imageReader.o: imageReader.c imageReader.h
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

terraview.o: terraview.cpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

clean:
	rm -rf $(OBJ) $(TARGETS)
