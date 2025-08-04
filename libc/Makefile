CFLAGS:=-O0 -g -ffreestanding -Wall -Wextra -Iinclude -std=gnu99

DEPS:=$(shell find . -type f -name '*.d')
FREEOBJS:=$(shell find . -type f -name '*.c')
FREEOBJS:=$(FREEOBJS:.c=.o)

OBJS=$(FREEOBJS)
LIBK_OBJS=$(FREEOBJS)

BINARIES=libk.a

.PHONY: all clean

all: $(BINARIES)

libk.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) -MD -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d *.a */*.o */*.d */*.a

-include $(DEPS)
