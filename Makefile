cc = gcc

firstdaemon: daemon.o student_vector.o
	$(cc) -o firstdaemon daemon.o student_vector.o

daemon.o: daemon.c student_vector.h
	$(cc) -c daemon.c student_vector.h

student_vector.o: student_vector.c student_vector.h
	$(cc) -c student_vector.c student_vector.h
	
clear:
	rm -r firstdaemon daemon.o student_vector.o student_vector.h.gch daemon.log
