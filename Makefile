cmake: wrapper cmake-real
	echo "#!$(CURDIR)/$<" > $@
	echo "set-env FOO=BAR" >> $@
	chmod +x $@

cmake-real: cmake.c
	$(CC) -o $@ $<

wrapper: wrapper.c
	$(CC) -o $@ $<

clean:
	rm cmake-real wrapper cmake
