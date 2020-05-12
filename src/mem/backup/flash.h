#ifndef GBA_FLASH_H
#define GBA_FLASH_H

#include "../gbamem.h"
#include "../gbabus.h"

void init_flash(gbamem_t* mem, backup_type_t type);
void write_byte_flash(gbamem_t* mem, gbabus_t* bus, word address, byte value);
byte read_byte_flash(gbamem_t* mem, word address, backup_type_t type);
#endif //GBA_FLASH_H
