#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    puts("Hello from cmake. Called with:");

    for (int i = 0; i < argc; ++i) {
        puts(argv[i]);
    }

    putchar('\n');

    char * foo = getenv("FOO");
    if (foo) {
        puts("FOO was set to:");
        puts(foo);
        putchar('\n');
    } else {
        puts("FOO was not set");
    }


    // Read a byte from stdin to halt
    puts("(waiting for stdin before exiting, this gives you time to look at ps aux/top)");
    char x;
    fread(&x, 1, 1, stdin);
}
