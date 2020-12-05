OPTS := -Wall -Wextra -pedantic -std=c11

all: maths_prog

%.o: %.c
	gcc $(OPTS) -fPIC -c $< -o $@

libmaths.so: add.o subtract.o
	gcc -shared add.o subtract.o -o libmaths.so

maths_prog: maths_prog.o libmaths.so
	gcc maths_prog.o -L./ -lmaths -o maths_prog

make clean:
	rm -f *.o *.so maths_prog