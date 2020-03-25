#include <cflags.h>

#include "gbarom.h"
#include "common/log.h"
#include "gbabus.h"
#include "arm7tdmi/arm7tdmi.h"
#include "gba_system.h"

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
    cflags_add_bool(flags, 'd', "debug", &debug, "enable debug mode");
    cflags_add_bool(flags, 's', "skip-bios", &should_skip_bios, "skip-bios");

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL, "enables verbose output, repeat up to 4 times for more verbosity");

    cflags_parse(flags, argc, argv);

    log_set_verbosity(verbose->count);

    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }

    gbamem_t* mem = init_mem();

    char* romfile = flags->argv[0];
    load_gbarom(romfile, mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi_t* cpu = init_arm7tdmi(gba_read_byte,
                                    gba_read_half,
                                    gba_read_word,
                                    gba_write_byte,
                                    gba_write_half,
                                    gba_write_word);
    gba_ppu_t* ppu = init_ppu();
    init_gbabus(mem, cpu, ppu);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    if (should_skip_bios) {
        logwarn("Skipping BIOS")
        skip_bios(cpu);
    }

    loginfo("Beginning CPU loop")

    while(true) {
        gba_system_step(cpu, ppu);
    }

    cflags_free(flags);
    return 0;
}