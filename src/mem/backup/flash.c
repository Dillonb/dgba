#include <stdlib.h>
#include <string.h>

#include "flash.h"
#include "../../common/log.h"

#define FLASH128_SIZE 131072
#define FLASH64_SIZE 65536

void init_flash(gbamem_t* mem, backup_type_t type) {
    switch (type) {
        case FLASH128K:
            mem->backup = malloc(FLASH128_SIZE);
            memset(mem->backup, 0, FLASH128_SIZE);
            break;
        case FLASH64K:
            mem->backup = malloc(FLASH64_SIZE);
            memset(mem->backup, 0, FLASH64_SIZE);
            break;
        default:
            logfatal("Called Flash function with incorrect backup type!")
    }
}

void write_byte_flash(gbamem_t* mem, word address, byte value, backup_type_t type) {
    logwarn("flash unimplemented: Tried to write 0x%02X to 0x%08X", value, address)
}

byte read_byte_flash(gbamem_t* mem, word address, backup_type_t type) {
    if (address == 0x0E000000) {
        return 0x62; // Stubbing flash
    } else if (address == 0x0E000001) {
        return 0x13; // Stubbing flash
    }
    return 0;
}
