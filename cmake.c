#include <stdlib.h>
#include <stdio.h>

int main() {
    puts("Hello from cmake");

    char * foo = getenv("FOO");
    if (foo) puts(foo);
    else puts("FOO was not set");

    // Read a byte from stdin to halt
    char x;
    fread(&x, 1, 1, stdin);
}
