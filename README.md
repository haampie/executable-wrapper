Simple executable wrapper that sets default variables, with original names in `ps aux` / `top`.


## Example

Build example with
```
$ make
```

This gives you `cmake`, which wraps `cmake-real` using `wrapper` as interpreter, modifying some variables
in a human-readable, hackable scripting language:

```
$ cat cmake
#!/tmp/tmp.HOMgkyakeP/wrapper
set FOO BAR
prepend PATH : /add/this/path
append CMAKE_PREFIX_PATH ; /first/path
append CMAKE_PREFIX_PATH ; /second/path
```

If you run it in a clean environment:

```
$ env -i PATH=/usr/bin ./cmake --hello --world
Hello from cmake. Called with:
./cmake
--hello
--world

PATH was set to:
/add/this/path:/usr/bin

CMAKE_PREFIX_PATH was set to:
/first/path;/second/path

(waiting for stdin before exiting, this gives you time to look at ps aux/top)
```

versus running the "real" binary directly:

```
$ env -i PATH=/usr/bin ./cmake-real --hello --world
Hello from cmake. Called with:
./cmake-real
--hello
--world

PATH was set to:
/usr/bin

CMAKE_PREFIX_PATH was not set
(waiting for stdin before exiting, this gives you time to look at ps aux/top)
```

## Env scripting language

The following commands are supported:

```
set <variable name> <value>
append <variable name> <delimiter> <value>
prepend <variable name> <delimiter> <value>
```

Inline comments are supported with `# <comment>`.

Variables, delimiters and values can be (a) literal values separated by whitespace:

```
set VARIABLE VALUE # VARIABLE is set to `VALUE`
set VARIABLE he"llo # VARAIBLE is set to `he"llo`
```

or (b) quoted strings:

```
set "VARIABLE" "VALUE" # VARIABLE is set to `VALUE`
```

or (c) delimited strings:

```
set r"(VARIABLE)" r"(xyz)"         # VARIABLE is set to `xyz`
set r"(VARIABLE)" r"(the "value")" # VARIABLE is set to `the "value"`
set r"(VARIABLE)" r"(((value)))"   # VARIABLE is set to `((value))`
```

Apart from `r"(...)"` it also supports `r"[...]"`, `r"{...}"` and `r"<...>"`.

NOTE: strings are not escaped, they're always literal.
If a variable or value contains whitespace, quotes, and all
of `)"`, `]"`, `}"`, `>"` you can't represent that string currently.\*

Multiline values require quoted or delimited strings:

```
set VARIABLE "the
value"  # VARIABLE is set to `the<newline>value`

set VARIABLE r"(the
value)"  # VARIABLE is set to `the<newline>value`
```

------

\* In principle I could still implement heredoc strings to work around
the limitation:

```
set VARIABLE r"EOS
this is the contents
EOS
```