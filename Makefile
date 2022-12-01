all: cmake-real wrapper

cmake-real: cmake.c
	$(CC) -o $@ $<

wrapper: wrapper.c
	$(CC) -o $@ $<

clean:
	rm cmake-real wrapper
