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

%-wrapped: print-hello.c
	$(CC) $(CFLAGS) -o $@ $<

test-basic: .test-basic-wrapped
	test $$(./$@) = "ccc;aaa:bbb"

test-quotes: .test-quotes-wrapped
	test $$(./$@) = "ccc;aaa:bbb"

test-empty-delim: .test-empty-delim-wrapped
	test $$(./$@) = "cccaaabbb"

test-delimited-strings: .test-delimited-strings-wrapped
	test $$(./$@) = "aaa:bbb:ccc:ddd"

test-heredoc-strings: .test-heredoc-strings-wrapped
	test $$(./$@) = 'aaa:b_"_b:c.".c:"xyz"'

test-syntax-error-string: .test-syntax-error-string-wrapped
	! ./$@ 2> /dev/null

test-syntax-error-delimited-string: .test-syntax-error-delimited-string-wrapped
	! ./$@ 2> /dev/null

clean:
	rm -f *-wrapped

