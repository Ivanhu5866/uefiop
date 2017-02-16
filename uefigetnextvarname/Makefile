CC      = gcc
CFLAGS  = -g -Wall -Werror
RM      = rm -f
INCDIR	= -I../include
INCLIB	= -L../lib
LIBS	= -lutils
BINDIR	= ../bin/

TARGETS := uefigetnextvarname

$(TARGETS): *.c
	@$(CC) $(CFLAGS) $< $(INCDIR) $(INCLIB) $(LIBS) -o $(BINDIR)$@

.PHONY: clean
clean:
	@$(RM) $(TARGETS)
