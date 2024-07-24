CC = gcc

TARGETS = aird cli
SUBDIRS = src src/aird src/cli src/kmod build

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -rf build/*.o
