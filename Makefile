cc = gcc
 
all: daemon.c cli.c c_vector.o common.o log.o
	$(cc) -o airdaemon daemon.c c_vector.o common.o log.o
	$(cc) -o cli cli.c common.o
 
c_vector.o: c_vector.c c_vector.h
	$(cc) -c c_vector.c c_vector.h

common.o: common.c common.h
	$(cc) -c common.c common.h

log.o: log.c log.h
	$(cc) -c log.c log.h
    
clean:
	rm -rf airdaemon daemon.o c_vector.o log.o c_vector.h.gch daemon.log error.log backup.log cli common.o common.h.gch log.h.gch
