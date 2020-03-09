#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/common/log.h"
#include "../src/arm7tdmi/arm7tdmi.h"
#include "../src/gbabus.h"
#include "../src/gbarom.h"
#include "../src/gbabios.h"

bool in_bios(arm7tdmi_t* state) {
    return state->pc < GBA_BIOS_SIZE;
}

int main(int argc, char** argv) {
    log_set_verbosity(4);
    gbamem_t* mem = init_mem();
    init_gbabus(mem);

    load_gbarom("arm.gba", mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi_t* cpu = init_arm7tdmi(gba_read_byte,
                                    gba_read_half,
                                    gba_read_word,
                                    gba_write_byte,
                                    gba_write_half,
                                    gba_write_word);
    skip_bios(cpu);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    bool bios_complete = false;
    while(true) {
        arm7tdmi_step(cpu);
        if (!bios_complete && !in_bios(cpu)) {
            bios_complete = true;
        }

        if (bios_complete) {
            word failed_test = cpu->r[12];
            if (failed_test != 0) {
                logfatal("test_arm: FAILED TEST: %d", failed_test)
            }
        }
        // TODO check for when all tests pass
    }
    exit(0);
}
