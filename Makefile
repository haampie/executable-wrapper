CFLAGS ?= -Os -Wall -Wextra -Wshadow -pedantic
EXECUTABLE_WRAPPER_CFLAGS = -std=c99
LDFLAGS ?=

.PHONY: all install clean

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datarootdir = $(prefix)/share
mandir = $(datarootdir)/man

all: executable-wrapper

executable-wrapper.o: executable-wrapper.c
	$(CC) $(CFLAGS) $(EXECUTABLE_WRAPPER_CFLAGS) -c -o $@ $<

executable-wrapper: executable-wrapper.o
	$(CC) $(LDFLAGS) -o $@ $<

install: all
	mkdir -p $(DESTDIR)$(bindir)
	cp -p executable-wrapper $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(mandir)/man1
	cp -p executable-wrapper.1 $(DESTDIR)$(mandir)/man1

check: all
	$(MAKE) -C test

clean:
	rm -f executable-wrapper.o executable-wrapper
