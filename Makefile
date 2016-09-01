SUBDIRS = lib uefivarset uefivarget uefitime

.PHONY: all clean

all:
	@for i in $(SUBDIRS); do \
	(cd $$i && make); \
	done
clean:
	rm -f bin/*
	@for i in $(SUBDIRS); do \
	(cd $$i && make clean); \
	done

