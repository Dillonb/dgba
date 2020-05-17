#include <cflags.h>

#include "mem/gbarom.h"
#include "gba_system.h"
#include "graphics/debug.h"
#include "graphics/render.h"

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
    int scale = 4;
    cflags_add_bool(flags, 'd', "debug", &debug, "enable debug mode at start");
    cflags_add_string(flags, 'b', "bios", &bios_file, "Alternative BIOS to load");
    cflags_add_bool(flags, 's', "skip-bios", &should_skip_bios, "Skip the bios, start execution at ROM entrypoint");
    cflags_add_int(flags, 'S', "scale", &scale, "Scale the screen (default 4)");

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL, "enables verbose output, repeat up to 4 times for more verbosity");

    cflags_parse(flags, argc, argv);
    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }

    log_set_verbosity(verbose->count);

    set_screen_scale(scale);


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