#ifndef GBA_FLASH_H
#define GBA_FLASH_H

#include "../gbamem.h"

void init_flash128k(gbamem_t* mem);
void init_flash64k(gbamem_t* mem);
void write_byte_flash128k(gbamem_t* mem, word address, byte value);
void write_byte_flash64k(gbamem_t* mem, word address, byte value);
byte read_byte_flash128k(gbamem_t* mem, word address);
byte read_byte_flash64k(gbamem_t* mem, word address);
#endif //GBA_FLASH_H
