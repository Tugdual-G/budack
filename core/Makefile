##
# Budack
#
# Variables
CC = mpicc
CFLAGS = -Wall -Wextra
LDFLAGS_main = -lm
LDFLAGS = -lm -ltiff -fPIC
LDLIBS = -L. -lslave -lmaster -lrender
# Targets
all: budack

budack: main_master_slaves.c libslave.so libmaster.so librender.so
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS) $(LDFLAGS_main)

libslave.so: slave.c budack_core.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) -O2

libmaster.so: master.c budack_core.c tiff_images.c librender.so
	$(CC) $(CFLAGS) -L. -shared -o $@ $^ $(LDFLAGS) -lrender -fopenmp -O0

librender.so: opengl/render.c opengl/compileShader.c opengl/display_window.c opengl/src/glad.c
	gcc $(CFLAGS) -I opengl -shared $^ -o $@ -lGL -lglfw -fPIC

clean:
	rm -f *.o budack *.so

.PHONY: clean
