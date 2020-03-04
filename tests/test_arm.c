#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/common/log.h"
#include "../src/arm7tdmi/arm7tdmi.h"
#include "../src/gbabus.h"
#include "../src/gbarom.h"

int main(int argc, char** argv) {
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

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    while(true) {
        arm7tdmi_step(cpu);
    }
    exit(0);
}
