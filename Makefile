CFLAGS ?= -Os -Wall -Wextra -Wshadow -pedantic
EXECUTABLE_WRAPPER_CFLAGS = -std=c99
LDFLAGS ?=

.PHONY: all install clean

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

all: executable-wrapper

executable-wrapper.o: executable-wrapper.c
	$(CC) $(CFLAGS) $(EXECUTABLE_WRAPPER_CFLAGS) -c -o $@ $<

executable-wrapper: executable-wrapper.o
	$(CC) $(LDFLAGS) -o $@ $<

install: all
	mkdir -p $(DESTDIR)$(bindir)
	cp -p executable-wrapper $(DESTDIR)$(bindir)

clean:
	rm -f executable-wrapper.o executable-wrapper
