mergesort: goodmalloc.o
	gcc -L. mergesort.c -lgoodmalloc -o sort

goodmalloc.o: goodmalloc.h goodmalloc.c
	gcc -c goodmalloc.c -lpthread
	ar rcs libgoodmalloc.a goodmalloc.o


clean:
	rm -f goodmalloc.o sort libgoodmalloc.a