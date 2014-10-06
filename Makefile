.PHONY: all test

CC = gcc
CFLAGS = -g -Wall -Werror -Wno-deprecated-declarations -Wno-unused-function -I. -fPIC

all: test libdmem.so

clean:
	rm -f */*.o */*_test.exe *.so

%.o: %.c dmem/*.h src/*.h
	$(CC) $(CFLAGS) -c $< -o $@

libdmem.so: src/vector.o src/char.o
	$(CC) $(CFLAGS) -shared $^ -o $@

%_test.exe: %_test.o libdmem.so
	$(CC) $(CFLAGS) -L. -ldmem $< -o $@
	./$@
	@echo TEST $@ ALL PASS

test: src/vector_test.exe src/char_test.exe

