##
# Budack
#
# Variables
CC = mpicc
CFLAGS = -Wall -Wextra
LDFLAGS_main =-lm '-Wl,-rpath,$$ORIGIN'
LDFLAGS = -lm -fPIC
LDLIBS = -L. -lslave -lmaster
# Targets
all: budack

budack: main_master_slaves.c libslave.so libmaster.so
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS) $(LDFLAGS_main)

libslave.so: slave.c budack_core.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) -O2

libmaster.so: master.c budack_core.c tiff_images.c opengl/src/glad.c opengl/display_window.c opengl/compileShader.c opengl/render.c
	$(CC) $(CFLAGS) -I opengl -L. -shared -o $@ $^ $(LDFLAGS) -ltiff -lGL -lglfw -O2


clean:
	rm -f *.o budack *.so

.PHONY: clean
