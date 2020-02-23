#include <err.h>
#include "gbabus.h"
#include "gbamem.h"
#include "log.h"

gbamem_t* mem;

void init_gbabus(gbamem_t* new_mem) {
    mem = new_mem;
}

void write_io_register(uint32_t addr, uint32_t value) {
    // 0x04XX0800 is the only address that's mirrored.
    if ((addr & 0xFF00FFFFu) == 0x04000800u) {
        addr = 0xFF00FFFFu;
    }

    logwarn("Wrote 0x%08X to 0x%08X", value, addr)
    unimplemented(1, "io register write")
}

byte gba_read_byte(uint32_t addr) {
    if (addr < 0x08000000) {
        logwarn("Tried to read from 0x%08X", addr)
        unimplemented(1, "Read from non-cartridge address")
    } else if (addr < 0x0E00FFFF) {
        // Cartridge
        uint32_t adjusted = addr - 0x08000000;
        if (adjusted > mem->rom_size) {
            logfatal("Attempted out of range read");
        } else {
            return mem->rom[adjusted];
        }
    }

    logfatal("Something's up, we reached the end of gba_read_byte without getting a value! addr: 0x%08X", addr)
}

uint16_t gba_read16(uint32_t addr) {
    byte lower = gba_read_byte(addr);
    byte upper = gba_read_byte(addr + 1);

    return (upper << 8u) | lower;
}

void gba_write_byte(uint32_t addr, byte value) {
    if (addr < 0x04000000) {
        logwarn("Tried to write to 0x%08X", addr)
        unimplemented(1, "Tried to write general internal memory")
    } else if (addr < 0x04000400) {
        write_io_register(addr, value);
    } else if (addr < 0x05000000) {
        logwarn("Tried to write to 0x%08X", addr)
        unimplemented(1, "Tried to write to unused portion of general internal memory")
    } else if (addr < 0x08000000) {
        logwarn("Tried to write to 0x%08X", addr)
        unimplemented(1, "Write to internal display memory address")
    } else if (addr < 0x0E00FFFF) {
        logwarn("Tried to write to 0x%08X", addr)
        unimplemented(1, "Write to cartridge address")
    } else {
        logfatal("Something's up, we reached the end of gba_write_byte without writing a value! addr: 0x%08X", addr)
    }
}

void gba_write16(uint32_t addr, uint16_t value) {
    byte lower = value & 0xFFu;
    byte upper = (value & 0xFF00u) >> 8u;
    gba_write_byte(addr, lower);
    gba_write_byte(addr + 1, upper);
}

uint32_t gba_read32(uint32_t addr) {
    uint32_t lower = gba_read16(addr);
    uint32_t upper = gba_read16(addr + 2);

    return (upper << 16u) | lower;
}

void gba_write32(uint32_t address, uint32_t value) {
    uint16_t lower = (value & 0xFFFFu);
    uint16_t upper = (value & 0xFFFF0000u);

    gba_write16(address, lower);
    gba_write16(address + 2, upper);
}

