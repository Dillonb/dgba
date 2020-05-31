#ifndef GBA_EEPROM_H
#define GBA_EEPROM_H

#include "../../common/util.h"
#include "../gbamem.h"

typedef enum eeprom_size {
    EEPROM_512,
    EEPROM_8K
} eeprom_size_t;

half read_half_eeprom(gbabus_t* bus, gbamem_t* mem, word address);
void write_half_eeprom(int active_dma, gbabus_t* bus, gbamem_t* mem, word address, half value);
#endif //GBA_EEPROM_H
