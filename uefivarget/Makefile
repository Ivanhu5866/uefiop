CC      = gcc
CFLAGS  = -g -Wall -Werror
RM      = rm -f
INCDIR	= -I../include
INCLIB	= -L../lib
LIBS	= -lutils

TARGETS := uefivarget

$(TARGETS): *.c
	@$(CC) $(CFLAGS) $< $(INCDIR) $(INCLIB) $(LIBS) -o $@

.PHONY: clean
clean:
	@$(RM) $(TARGETS)
