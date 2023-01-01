#include <stdio.h>
#include <stdlib.h>

int main() {
    char * result = getenv("HELLO");
    if (result != NULL) puts(result);
}
