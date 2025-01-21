CFLAGS:=--sysroot=$(SYSROOT_USER) -isystem=$(INCLUDEDIR) -O0 -g -fno-stack-protector -ffreestanding -Wall -Wextra -nostdlib

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
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) *.d *.a */*.o */*.d */*.a

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR_USER)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR_USER)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR_USER)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR_USER)$(LIBDIR)/.

-include $(DEPS)
