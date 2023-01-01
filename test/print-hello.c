#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char * result = getenv("HELLO");
    if (result != NULL) puts(result);
}
