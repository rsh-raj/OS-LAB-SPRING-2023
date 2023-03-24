a.out:functions.h global_data.h mutex_locks.h semaphores.h staff.o guest.o main.cpp
	g++ -I. main.cpp guest.o staff.o -lpthread
	./a.out

staff.o:functions.h global_data.h mutex_locks.h semaphores.h staff.cpp
	g++ -c -I. staff.cpp

guest.o:functions.h global_data.h mutex_locks.h semaphores.h guest.cpp
	g++ -c -I. guest.cpp 

clean:
	rm -f a.out staff.o guest.o