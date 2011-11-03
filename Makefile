.PHONY: all

CFLAGS = -O2 -Wall -Wno-deprecated-declarations -Wno-unused-function -I. -D_GNU_SOURCE -fPIC

all:
	rm -rf *.o
	gcc $(CFLAGS) -c src/*.c
	gcc $(CFLAGS) -lz -luuid -shared *.o -o libdmem.so
