#include <stdlib.h>
#include <string.h>

#include "flash.h"
#include "../../common/log.h"

#define FLASH128_SIZE 131072
#define FLASH64_SIZE 65536

void init_flash128k(gbamem_t* mem) {
    mem->backup = malloc(FLASH128_SIZE);
    memset(mem->backup, 0, FLASH128_SIZE);
}

void init_flash64k(gbamem_t* mem) {
    mem->backup = malloc(FLASH64_SIZE);
    memset(mem->backup, 0, FLASH64_SIZE);
}

void write_byte_flash128k(gbamem_t* mem, word address, byte value) {
    logwarn("flash 128k unimplemented: Tried to write 0x%02X to 0x%08X", value, address)
}

void write_byte_flash64k(gbamem_t* mem, word address, byte value) {
    logwarn("flash 64k unimplemented: Tried to write 0x%02X to 0x%08X", value, address)
}

byte read_byte_flash128k(gbamem_t* mem, word address) {
    if (address == 0x0E000000) {
        return 0x62; // Stubbing flash
    } else if (address == 0x0E000001) {
        return 0x13; // Stubbing flash
    }
    return 0;
}

byte read_byte_flash64k(gbamem_t* mem, word address) {
    if (address == 0x0E000000) {
        return 0x62; // Stubbing flash
    } else if (address == 0x0E000001) {
        return 0x13; // Stubbing flash
    }
    return 0;
}

