HOST?=$(DEFAULT_HOST)
ARCHDIR:=arch/$(HOSTARCH)

# define flags if not already defined
CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
ASFLAGS?=
LIBS?=

# add extra flags if needed
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude -I../kernel/include
LDFLAGS:=$(LDFLAGS)
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

DEPS:=$(shell find . -type f -name '*.d')
FREEOBJS:=$(shell find . -type f -name '*.c')
FREEOBJS:=$(FREEOBJS:.c=.o)

OBJS=\
$(FREEOBJS)

LIBK_OBJS=$(FREEOBJS)

BINARIES=libk.a

.PHONY: all clean install install-headers install-kernel

all: $(BINARIES)

libk.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d *.a */*.o */*.d */*.a

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)/.

-include $(DEPS)
