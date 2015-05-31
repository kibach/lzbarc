all:
	gcc -o lzbarc -std=gnu99 -pedantic -lm -L. lz77.c bitvector.c dotlzb.c lzbarc.c
test:
	gcc -c -std=gnu99 -o bitvector.o bitvector.c
	gcc -c -std=gnu99 -o dotlzb.o dotlzb.c
	gcc -c -std=gnu99 -o lz77.o lz77.c
	gcc -c -o bitvectest.o bitvectest.c
	gcc -c -o mutest.o mutest.c
	mkmutest mutest.h bitvector.o bitvectest.o | cc -xc -c -o runmutest.o -
	gcc -o bv11 mutest.o bitvector.o bitvectest.o runmutest.o -lm
	./bv11 -v
