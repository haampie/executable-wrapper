#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    puts("Hello from cmake. Called with:");

    for (int i = 0; i < argc; ++i) {
        puts(argv[i]);
    }

    putchar('\n');

    char * path = getenv("PATH");
    if (path) {
        puts("PATH was set to:");
        puts(path);
        putchar('\n');
    } else {
        puts("PATH was not set");
    }


    char * cmake_prefix_path = getenv("CMAKE_PREFIX_PATH");
    if (cmake_prefix_path) {
        puts("CMAKE_PREFIX_PATH was set to:");
        puts(cmake_prefix_path);
        putchar('\n');
    } else {
        puts("CMAKE_PREFIX_PATH was not set");
    }


    char * delimiters = getenv("DELIMITERS");
    if (delimiters) {
        puts("DELIMITERS was set to:");
        puts(delimiters);
        putchar('\n');
    } else {
        puts("DELIMITERS was not set");
    }

    // Read a byte from stdin to halt
    puts("(waiting for stdin before exiting, this gives you time to look at ps aux/top)");
    char x;
    fread(&x, 1, 1, stdin);
}
