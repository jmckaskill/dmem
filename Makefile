.PHONY: all test

CC = gcc
AR = ar
CFLAGS = -g -Wall -Werror -Wno-deprecated-declarations -Wno-unused-function -I. -fPIC

all: test libdmem.a

clean:
	rm -f */*.o */*_test.exe *.so *.a

%.o: %.c dmem/*.h src/*.h
	$(CC) $(CFLAGS) -c $< -o $@

libdmem.so: src/vector.o src/char.o
	$(CC) $(CFLAGS) -shared $^ -o $@

libdmem.a: src/vector.o src/char.o
	$(AR) rcs $@ $^

%_test.exe: %_test.o libdmem.a
	$(CC) $(CFLAGS) -L. -ldmem $< -o $@
	./$@
	@echo TEST $@ ALL PASS

test: src/vector_test.exe src/char_test.exe

