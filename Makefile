SUBLIB = lib
SUBDIRS = uefivarset uefivarget uefitime uefigetnextvarname uefiresetsystem
INSTALL = install
prefix = /usr
LIBDIR = $(prefix)/lib
DESTBIN = $(prefix)/bin

.PHONY: all clean

all:
	(cd $(SUBLIB) && make);
	@for i in $(SUBDIRS); do \
	(cd $$i && make); \
	done
clean:
	rm -f bin/*
	(cd $(SUBLIB) && make clean);
	@for i in $(SUBDIRS); do \
	(cd $$i && make clean); \
	done

install:
	@for i in $(SUBDIRS); do \
	$(INSTALL) -m 755 bin/$$i $(DESTDIR)$(DESTBIN); \
	done
