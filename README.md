Simple executable wrapper for managing environment variables before execution.

## Motivation

In package managers like [Spack](https://github.com/spack/spack/) it is
convenient to replace a Python executable in environments with a wrapper that
sets relevant variables such as `PYTHONPATH`, so that Python can immediately
locate modules.

Similarly in build environments it's nice to be able to execute `cmake`, `gcc`
etc with the correct variables. In particular when dealing with hermetic build
systems that clear environment variables (and don't play nice with non-standard
filesystem structures like Spack has) it's nice to use wrappers for executables
of build dependencies.

This project provides a very basic scripting language to manage variables,
which can be more convenient than shell scripts:

```
#!/path/to/executable-wrapper
set FOO BAR
prepend PATH : /some/path/bin
append CMAKE_PREFIX_PATH ; /some/other/path
```

Given an executable `hello`, you can simply move the executable to `hello-real`
and replace it with a script:

```
$ mv hello hello-real
$ vim hello # write the script
$ ./hello # execute hello-real with modified variables
```

## How to build

```
make
make install prefix=/usr/local
```

## Commands / syntax

The following commands are supported:

```
set <variable name> <value>
append <variable name> <delimiter> <value>
prepend <variable name> <delimiter> <value>
```

Variables, delimiters and values are **strings**.

Inline comments are supported with `# <comment>`.

### String types

Strings are **always** raw and there is no such thing as an escape character.
The reason not to have an escape character is mostly because `\` itself is used
as a directory separator on some infamous platforms, and it's not uncommon to
end a string with `\`, which is awkward (in Python you cannot end a raw string
with a literal backslash, oops!). Performance-wise it's also nice not to have
escape characters, since it requires fewer copies.

The simplest type of string is a *literal string*,
which is always separated by whitespace.

```
set VARIABLE VALUE  # VARIABLE is set to `VALUE`
set VARIABLE he"llo # VARIABLE is set to `he"llo`
```

Then there are *quoted strings* using double quotes:

```
set "VARIABLE" "VALUE" # VARIABLE is set to `VALUE`
```

And finally there are *delimited* strings:

```
set r"(VARIABLE)" r"(xyz)"         # VARIABLE is set to `xyz`
set r"(VARIABLE)" r"(the "value")" # VARIABLE is set to `the "value"`
set r"(VARIABLE)" r"(((value)))"   # VARIABLE is set to `((value))`
```

Apart from `r"(...)"` it also supports `r"[...]"`, `r"{...}"` and `r"<...>"`.

It also supports custom heredoc-like delimiters:

```
set VARIABLE r"✌️(value)✌️"                   # set VARIABLE to `value`
set DELIMITERS r"test{)", ]", }", >"}test"  # set DELIMITERS to `)", ]", }", >"`
```

The heredoc delimiter can be any length and contain any byte but `(`, `[`, `{` and `<`.

### Multiline strings

In quoted and delimited strings new line characters are literal too, and
can be used to create multiline strings:

```
set VARIABLE "the
value"  # VARIABLE is set to `the<newline>value`

set VARIABLE r"(the
value)"  # VARIABLE is set to `the<newline>value`
```
