mergesort: goodmalloc.o
	ar rcs libgoodmalloc.a goodmalloc.o
	gcc -L. mergesort.c -lgoodmalloc -o sort

goodmalloc.o: goodmalloc.h goodmalloc.c
	gcc -c goodmalloc.c

clean:
	rm -f goodmalloc.o mergesort libgoodmalloc.a