liblookup.a: lookup.o
	ar rcs liblookup.a lookup.o

lookup.o: lookup.h lookup.c
	gcc -Wall -march=native -O3 -c lookup.c

.phony: clean

clean:
	rm -f *.o *.a
