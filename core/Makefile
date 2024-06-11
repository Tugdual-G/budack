##
# Budack
#
# Variables
CC = mpicc
CFLAGS = -Wall -Wextra
LDFLAGS_main = -lm
LDFLAGS = -lm -ltiff -fPIC
LDLIBS = -L. -lslave -lmaster

# Targets
all: budack

budack: main_master_slaves.c libslave.so libmaster.so
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS) $(LDFLAGS_main)

libslave.so: slave.c budack_core.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

libmaster.so: master.c budack_core.c tiff_images.c
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o budack libslave.so libmaster.so

.PHONY: clean
