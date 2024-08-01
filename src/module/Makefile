obj-m += batmodule.o

DIREC = $(shell pwd)
OBJS = batmodule.ko batmodule.mod batmodule.mod.c batmodule.mod.o batmodule.o modules.order Module.symvers
TARGETDIR = ../../build/kbuild

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(DIREC) modules
	$(MAKE) move

move: $(OBJS)
	mkdir -p $(TARGETDIR)
	for file in $(OBJS); do \
		mv -f $(DIREC)/$$file $(TARGETDIR); \
	done

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(DIREC) clean
	rm -rf $(TARGETDIR)
