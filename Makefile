CC = gcc

TARGETS = aird cli
SUBDIRS = src src/daemon src/cli src/module build

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -rf build/*.o
