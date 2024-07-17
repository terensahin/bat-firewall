cc = gcc
 
all: daemon.c c_vector.o common.o cli.c
	$(cc) -o airdaemon daemon.c c_vector.o common.o
	$(cc) -o cli cli.c common.o
 
c_vector.o: c_vector.c c_vector.h
	$(cc) -c c_vector.c c_vector.h

common.o: common.c common.h
	$(cc) -c common.c common.h
    
clean:
	rm -rf airdaemon daemon.o c_vector.o c_vector.h.gch daemon.log error.log backup.log cli common.o common.h.gch
