##
# Project Title
#
# @file
# @version 0.1
#
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lGL -lglfw -fPIC
LDLIBS = -L. -lrender

all: main

main: main.c librender.so
	gcc $(CFLAGS) $< -o $@ $(LDLIBS)

librender.so: render.c compileShader.c display_window.c src/glad.c
	gcc $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

run:
	./main

clean:
	rm *.so main
# end
