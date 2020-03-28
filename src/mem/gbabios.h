#ifndef GBA_BIOS_H
#define GBA_BIOS_H

#include "../common/util.h"

#define GBA_BIOS_SIZE 0x4000

byte gbabios_read_byte(word address);
void load_alternate_bios(const char* filename);

#endif //GBA_BIOS_H
