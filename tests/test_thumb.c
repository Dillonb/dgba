#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/gbarom.h"
#include "../src/gbabus.h"
#include "../src/arm7tdmi/arm7tdmi.h"
#include "../src/common/log.h"

int main(int argc, char** argv) {
    gbamem_t* mem = init_mem();

    load_gbarom("thumb.gba", mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi_t* cpu = init_arm7tdmi(gba_read_byte,
                                    gba_read_half,
                                    gba_read_word,
                                    gba_write_byte,
                                    gba_write_half,
                                    gba_write_word);

    init_gbabus(mem, cpu);
    skip_bios(cpu);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    while(true) {
        arm7tdmi_step(cpu);
        word failed_test = cpu->r[7];
        if (failed_test != 0) {
            logfatal("test_thumb: FAILED TEST: %d", failed_test)
        }
        // TODO check for when all tests pass
    }
    exit(0);
}
