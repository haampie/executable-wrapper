#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    // The wrapper file
    char *name = argv[1];
    FILE *f = fopen(name, "rb");
    if (f == NULL) return 1;

    // Get the real exe: [wrapper]-real
    char real_exe[4096];
    int file_len = strlen(name);
    memcpy(real_exe, name, file_len);
    strcpy(real_exe + file_len, "-real");

    // Read the contents and set variables
    size_t len = 0;
    ssize_t nread;
    char * line;
    for (size_t i = 0; (nread = getline(&line, &len, f)) != -1; ++i) {
        // skip the shebang
        if (i == 0) continue;

        // parse set-env command
        if (strncmp(line, "set-env ", 8) != 0)
            continue;

        // extract the env key / value and set with overwrite
        line[nread-1] = '\0';
        char * separator = strchr(line, '=');
        if (separator == NULL) continue;
        *separator = '\0';
        setenv(line + 8, separator + 1, 1);
    }

    execv(real_exe, argv + 1);
}
