CFLAGS ?= -Os -Wall

TESTS += test-basic
TESTS += test-quotes
TESTS += test-empty-delim
TESTS += test-delimited-strings
TESTS += test-heredoc-strings
TESTS += test-syntax-error-string
TESTS += test-syntax-error-delimited-string

.PHONY: $(TESTS)

check: $(TESTS)

%-real: print-hello.c
	$(CC) $(CFLAGS) -o $@ $<

test-basic: test-basic-real
	test $$(./$@) = "ccc;aaa:bbb"

test-quotes: test-quotes-real
	test $$(./$@) = "ccc;aaa:bbb"

test-empty-delim: test-empty-delim-real
	test $$(./$@) = "cccaaabbb"

test-delimited-strings: test-delimited-strings-real
	test $$(./$@) = "aaa:bbb:ccc:ddd"

test-heredoc-strings: test-heredoc-strings-real
	test $$(./$@) = 'aaa:b_"_b:c.".c:"xyz"'

test-syntax-error-string: test-syntax-error-string-real
	! ./$@ 2> /dev/null

test-syntax-error-delimited-string: test-syntax-error-delimited-string-real
	! ./$@ 2> /dev/null

clean:
	rm -f *-real

