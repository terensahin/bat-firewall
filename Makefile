cc = gcc
 
all: daemon.o c_dynamic_vector.o cli.c
	$(cc) -o airdaemon daemon.o c_dynamic_vector.o
	$(cc) -o cli cli.c
 
daemon.o: daemon.c c_dynamic_vector.h
	$(cc) -c daemon.c c_dynamic_vector.h
 
c_dynamic_vector.o: c_dynamic_vector.c c_dynamic_vector.h
	$(cc) -c c_dynamic_vector.c c_dynamic_vector.h
    
clean:
	rm -r airdaemon daemon.o c_dynamic_vector.o c_dynamic_vector.h.gch daemon.log cli