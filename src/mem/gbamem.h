#ifndef __GBAMEM_H__
#define __GBAMEM_H__

#include <stddef.h>
#include <stdbool.h>

#include "../common/util.h"
#include "gbabus.h"

// RAM (inside GBA)
#define EWRAM_SIZE 0x40000
#define IWRAM_SIZE 0x8000

// Backup (on cartridge)
#define SRAM_SIZE 0x8000

typedef enum flash_state {
    FLASH_READY,
    FLASH_CMD_1,
    FLASH_CMD_2,
    FLASH_CHIP_ID,
    FLASH_ERASE,
    FLASH_ERASE_1,
    FLASH_ERASE_2,
    FLASH_ERASE_WRITE_SECTOR,
    FLASH_WRITE_SINGLE_BYTE,
    FLASH_BANKSWITCH
} flash_state_t;

typedef enum eeprom_state {
    EEPROM_READY,
    EEPROM_CMD_1,
    EEPROM_READ,
    EEPROM_WRITE,
    EEPROM_POST_WRITE,
    EEPROM_ACCEPT_READ_ADDRESS,
    EEPROM_ACCEPT_WRITE_ADDRESS
} eeprom_state_t;

typedef struct gbamem {
    byte* rom;
    size_t rom_size;
    byte ewram[EWRAM_SIZE];
    byte iwram[IWRAM_SIZE];
    size_t backup_size;
    byte* backup;
    bool backup_dirty;
    const char* backup_path;
    const char** savestate_path;

    // Flash state
    flash_state_t flash_state;
    word flash_command;
    byte flash_bank;
    word flash_erase_write_sector_first_address;
    int backup_persist_countdown;

    // EEPROM state
    eeprom_state_t eeprom_state;
    byte eeprom_command;
    int eeprom_bits_remaining;
    bool eeprom_initialized;
    half eeprom_address;
} gbamem_t;

gbamem_t* init_mem();

#endif
