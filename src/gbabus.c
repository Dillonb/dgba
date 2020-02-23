#include <err.h>
#include "gbabus.h"
#include "gbamem.h"
#include "log.h"

gbamem_t* mem;

void init_gbabus(gbamem_t* new_mem) {
    mem = new_mem;
}

byte gba_read_byte(uint32_t addr) {
    if (addr < 0x08000000) {
        // TODO
    } else if (addr < 0x0E00FFFF) {
        // Cartridge
        uint32_t adjusted = addr - 0x08000000;
        if (adjusted > mem->rom_size) {
            logfatal("Attempted out of range read");
        }
        else {
            return mem->rom[adjusted];
        }
    }

    logwarn("Something's up, we reached the end of gba_read_byte without getting a value\n");
    return 0; // TODO
}

uint16_t gba_read16(uint32_t addr) {
    byte lower = gba_read_byte(addr);
    byte upper = gba_read_byte(addr + 1);

    return (upper << 8u) | lower;
}

void gba_write_byte(uint32_t addr, byte value) {

}

void gba_write16(uint32_t addr, uint16_t value) {

}
