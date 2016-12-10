CC = g++
CFLAGS = -fno-builtin -Wall -Wno-deprecated -O3 -c
LDFLAGS = -O3 -o
HEADERPATH =
LIBPATH =
LIBS = -lm
OBJ = SceneReader.o data_types.o lights.o objects.o \
      Camera.o quadrics.o planes.o box.o Pixmap.o kiran.o

TARGETS = kiran

target: $(TARGETS)

kiran: $(OBJ)
	$(CC) $(LDFLAGS) $@ $(OBJ) $(LIBPATH) $(LIBS)

SceneReader.o: SceneReader.cpp SceneReader.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

data_types.o: data_types.cpp data_types.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

lights.o: lights.cpp lights.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

objects.o: objects.cpp objects.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)
	
quadrics.o: quadrics.cpp objects.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

planes.o: planes.cpp objects.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

box.o: box.cpp objects.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

Pixmap.o: Pixmap.cpp Pixmap.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

Camera.o: Camera.cpp Camera.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

kiran.o: kiran.cpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

clean:
	rm -rf $(OBJ) $(TARGETS) output.ppm
