CC = g++
CFLAGS = -fno-builtin -Wall -Wno-deprecated -O3 -c
LDFLAGS = -O3 -o
HEADERPATH =
LIBPATH =
LIBS = -lm
OBJ = data_types.o objects.o Pixmap.o kiran.o \
      SceneReader.o lights.o camera.o

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
	
Pixmap.o: Pixmap.cpp Pixmap.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

camera.o: camera.cpp camera.hpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

kiran.o: kiran.cpp
	$(CC) -o $@ $(CFLAGS) $< $(HEADERPATH)

clean:
	rm -rf $(OBJ) $(TARGETS) output.ppm
