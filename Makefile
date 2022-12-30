cmake: wrapper cmake-real
	echo "#!$(CURDIR)/$<" > $@
	echo "set FOO BAR" >> $@
	echo "prepend PATH : /add/this/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /first/path" >> $@
	echo "append CMAKE_PREFIX_PATH ; /second/path" >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"(/this/"path"/here)"' >> $@
	echo 'append CMAKE_PREFIX_PATH ; r"hello(/third/path)hello"' >> $@
	echo 'set DELIMITERS r"test()", ]", }", >")test"' >> $@
	chmod +x $@

cmake-real: cmake.c
	$(CC) -o $@ $<

wrapper: wrapper.c
	$(CC) -Os -Wl,-s -o $@ $<

clean:
	rm cmake-real wrapper cmake
