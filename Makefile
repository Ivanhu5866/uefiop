SUBDIRS = lib uefivarset

.PHONY: all clean

all:
	@for i in $(SUBDIRS); do \
	(cd $$i && make); \
	done
clean:
	@for i in $(SUBDIRS); do \
	(cd $$i && make clean); \
	done

