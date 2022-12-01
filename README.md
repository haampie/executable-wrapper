Simple executable wrapper that sets defaults variables, with original names in `ps aux` / `top` names.

```
$ make
$ ./cmake --hello --world
Hello from cmake. Called with:
./cmake
--hello
--world

FOO was set to:
BAR

(waiting for stdin before exiting, this gives you time to look at ps aux/top)
```

