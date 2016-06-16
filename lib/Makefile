CC      = gcc
CFLAGS  = -c
RM      = rm -f
AR	= ar crs

TARGETS := libutils.a

$(TARGETS): *.o
	@$(AR) $@ $<

%.o: %.c
	@$(CC) $(CFLAGS) $^

.PHONY: clean
clean:
	@$(RM) $(TARGETS) *.o
