Simple executable wrapper that sets defaults variables, with original names in `ps aux` / `top`.

Build example with
```
$ make
```

This gives you `cmake`, which wraps `cmake-real` using `wrapper` as interpreter, setting a few variables:

```
$ cat cmake
#!/tmp/tmp.HOMgkyakeP/wrapper
set-env FOO=BAR
```

If you run it, you get:

```
$ ./cmake --hello --world
Hello from cmake. Called with:
./cmake
--hello
--world

FOO was set to:
BAR

(waiting for stdin before exiting, this gives you time to look at ps aux/top)
```

versus running the "real" binary directly:

```
$ ./cmake-real --hello --world
Hello from cmake. Called with:
./cmake-real
--hello
--world

FOO was not set
(waiting for stdin before exiting, this gives you time to look at ps aux/top)
```

