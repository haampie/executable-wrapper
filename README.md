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

Variables, delimiters and values can be literal values separated by whitespace:

```
set VARIABLE_NAME VALUE
```

or quoted strings:

```
set "VARIABLE_NAME" "VALUE"
```

or delimited strings:

```
set r"(VARIABLE_NAME)" r"(VALUE)"
```
