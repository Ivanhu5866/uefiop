CC      = gcc
CFLAGS  = -c
RM      = rm -f
AR	= ar crs
INCDIR	= -I../include

TARGETS := libutils.a

$(TARGETS): *.o
	@$(AR) $@ $<

%.o: %.c
	@$(CC) $(CFLAGS) $^ $(INCDIR)

.PHONY: clean
clean:
	@$(RM) $(TARGETS) *.o
