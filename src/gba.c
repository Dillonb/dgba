#include <cflags.h>

#include "gbarom.h"
#include "log.h"
#include "gbabus.h"
#include "arm7tdmi.h"

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

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL, "enables verbose output, repeat up to 4 times for more verbosity");

    cflags_parse(flags, argc, argv);

    LOG_set_verbosity(verbose->count);

    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }

    gbamem* mem = init_mem();
    init_gbabus(mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi* cpu = init_arm7tdmi(gba_read_byte, gba_read16, gba_write_byte, gba_write16);

    char* romfile = flags->argv[0];
    load_gbarom(romfile, mem);

    LOG(INFO, "ROM loaded: %d bytes", mem->rom_size);
    LOG(INFO, "Beginning CPU loop");

    while(true) {
        arm7tdmi_tick(cpu);

    }



    cflags_free(flags);
    return 0;
}