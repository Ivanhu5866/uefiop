CC      = gcc
CFLAGS  = -g -Wall -Werror
RM      = rm -f

TARGETS := uefivarset

$(TARGETS): *.c
	@$(CC) $(CFLAGS) $< -I../include -o $@

clean:
	@$(RM) uefivarset
