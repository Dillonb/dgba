#include <stdio.h>
#include <cflags.h>

#include "rom.h"

void usage(cflags_t* flags) {
    cflags_print_usage(flags,
                       "[OPTION]... FILE",
                       "gba, a dgb gba emulator",
                       "https://github.com/Dillonb/gba");
}

int main(int argc, char** argv) {
    cflags_t* flags = cflags_init();
    bool debug = false;
    cflags_add_bool(flags, 'd', "debug", &debug, "enable debug mode");

    cflags_parse(flags, argc, argv);

    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }
    char* romfile = flags->argv[0];
    load_gbarom(romfile);

    printf("Hi\n");

    cflags_free(flags);
    return 0;
}