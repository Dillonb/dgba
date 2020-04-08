#include <cflags.h>

#include "mem/gbarom.h"
#include "common/log.h"
#include "mem/gbabus.h"
#include "arm7tdmi/arm7tdmi.h"
#include "gba_system.h"
#include "graphics/debug.h"
#include "mem/gbabios.h"

void usage(cflags_t* flags) {
    cflags_print_usage(flags,
                       "[OPTION]... FILE",
                       "gba, a dgb gba emulator",
                       "https://github.com/Dillonb/gba");
}

int main(int argc, char** argv) {
    cflags_t* flags = cflags_init();
    bool debug = false;
    bool should_skip_bios = false;
    const char* bios_file = NULL;
    cflags_add_bool(flags, 'd', "debug", &debug, "enable debug mode at start");
    cflags_add_string(flags, 'b', "bios", &bios_file, "Alternative BIOS to load");
    cflags_add_bool(flags, 's', "skip-bios", &should_skip_bios, "skip-bios");

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL, "enables verbose output, repeat up to 4 times for more verbosity");

    cflags_parse(flags, argc, argv);

    log_set_verbosity(verbose->count);

    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }

    init_gbasystem(flags->argv[0], bios_file);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    if (should_skip_bios) {
        logwarn("Skipping BIOS")
        skip_bios(cpu);
    }

    loginfo("Beginning CPU loop")

    if (debug) {
        set_dbg_window_visibility(true);
    }

    gba_system_loop(cpu, ppu, bus);

    cflags_free(flags);
    return 0;
}