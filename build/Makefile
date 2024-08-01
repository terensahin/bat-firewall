CC = gcc
CFLAGS = -g -Wall -Wextra
DAEMONOBJS = airbatd.o common.o c_vector.o log.o
DAEMONTARGET = airbatd
CLIOBJS = airbat_cli.o common.o
CLITARGET = airbat_cli

.PHONY: all clean

all: $(DAEMONTARGET) $(CLITARGET)

$(DAEMONTARGET): $(DAEMONOBJS)
	$(CC) $(CFLAGS) -o $(DAEMONTARGET) $(DAEMONOBJS)

$(CLITARGET): $(CLIOBJS)
	$(CC) $(CFLAGS) -o $(CLITARGET) $(CLIOBJS)

clean:
	rm -rf *.o $(DAEMONTARGET) $(CLITARGET)
