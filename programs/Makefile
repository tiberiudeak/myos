# define flags if not already defined
CFLAGS:=--sysroot=$(SYSROOT_USER) -isystem=$(INCLUDEDIR) -O0 -g -ffreestanding -Wall -fPIE
CPPFLAGS:=
# link with my libc
LDFLAGS:=-L$(SYSROOT_USER)

# add extra flags if needed
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk

EXEC_SOURCES_C:=$(shell find . -type f -name '*.c')
KERNEL_DEPS:=$(shell find . -type f -name '*.d')

EXEC_OBJS:=$(EXEC_SOURCES_C:.c=.o)

OBJS=\
$(EXEC_OBJS)

BINARIES:=$(foreach obj,$(OBJS),$(basename $(obj)))

.PHONY: all clean install
.SUFFIXES: .o .c

all: $(BINARIES)

$(BINARIES): %: %.o
	$(LD) $(LDFLAGS) $< -o $@ $(LIBS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

install: $(BINARIES)
	mkdir -p ../$(BIN_DIR_USER)
	cp $(BINARIES) ../$(BIN_DIR_USER)

clean:
	rm -rf $(BINARIES) $(OBJS) *.d */*.d */*/*.d

