CFLAGS ?= -Os

cmake: cmake-real
	echo "#!$(CURDIR)/../executable-wrapper" > $@
	echo "set FOO BAR" >> $@
	echo "prepend PATH : /add/this/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /first/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /second/path" >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"(/this/"path"/here)"' >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"_____(/third/path)_____"' >> $@
	echo 'set DELIMITERS r"EOS[)", ]", }", >"]EOS"' >> $@
	chmod +x $@

cmake-real: cmake.c
	$(CC) $(CFLAGS) -o $@ $<

