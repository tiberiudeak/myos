
HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS:=-O0 -g
CPPFLAGS:=
# link with my libc
LDFLAGS:=-L../sysroot/usr/lib/
ASFLAGS:=
LIBS:=

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fPIE
CPPFLAGS:=$(CPPFLAGS)
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk

EXEC_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

EXEC_OBJS:=$(EXEC_SOURCES_C:.c=.o)

OBJS=\
$(EXEC_OBJS)

BINARIES:=$(foreach obj,$(OBJS),$(basename $(obj)))

.PHONY: all clean
.SUFFIXES: .o .c

all: $(BINARIES)

$(BINARIES): $(OBJS)
	$(LD) $(LDFLAGS) $< -o $@ $(LIBS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d */*.d */*/*.d

