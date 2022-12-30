CFLAGS ?= -Os -Wall -Wextra -Wshadow -pedantic
EXECUTABLE_WRAPPER_CFLAGS = -std=c99
LDFLAGS ?=

all: executable-wrapper

executable-wrapper.o: executable-wrapper.c
	$(CC) $(CFLAGS) $(EXECUTABLE_WRAPPER_CFLAGS) -c -o $@ $<

executable-wrapper: executable-wrapper.o
	$(CC) $(LDFLAGS) -o $@ $<

cmake: wrapper cmake-real
	echo "#!$(CURDIR)/$<" > $@
	echo "set FOO BAR" >> $@
	echo "prepend PATH : /add/this/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /first/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /second/path" >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"(/this/"path"/here)"' >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"_____(/third/path)_____"' >> $@
	echo 'set DELIMITERS r"EOS[)", ]", }", >"]EOS"' >> $@
	chmod +x $@

cmake-real: cmake.c
	$(CC) -o $@ $<

clean:
	rm -f executable-wrapper.o executable-wrapper
