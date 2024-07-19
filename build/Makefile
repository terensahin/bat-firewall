CC = gcc
CFLAGS = -g -Wall
DAEMONOBJS = aird.o common.o c_vector.o log.o
DAEMONTARGET = aird
CLIOBJS = cli.o common.o
CLITARGET = cli

.PHONY: all clean

all: $(DAEMONTARGET) $(CLITARGET)

$(DAEMONTARGET): $(DAEMONOBJS)
	$(CC) $(CFLAGS) -o $(DAEMONTARGET) $(DAEMONOBJS)

$(CLITARGET): $(CLIOBJS)
	$(CC) $(CFLAGS) -o $(CLITARGET) $(CLIOBJS)

clean:
	rm -rf *.o $(DAEMONTARGET) $(CLITARGET)
