#include <stdlib.h>
#include <string.h>

#include "flash.h"

#define FLASH128_SIZE 0x20000
#define FLASH64_SIZE  0x10000

#define ADDRESS13 0xE005555
#define ADDRESS2  0xE002AAA

#define DEVICE 0x13
#define MANUFACTURER 0x62

#define FLASHC_CHIP_ID 0xAA5590
#define FLASHC_EXIT_CHIP_ID 0xAA55F0
#define FLASHC_BANKSWITCH 0xAA55B0
#define FLASHC_ERASE 0xAA5580
#define FLASHC_ERASE_ENTIRE_CHIP 0xAA5510
// Either erase and write 128 byte sector
// or write single data byte
// depending on which flash memory controller is used.
#define FLASHC_WRITE_DATA 0xAA55A0


void init_flash(gbamem_t* mem, backup_type_t type) {
    switch (type) {
        case FLASH128K:
            mem->backup = malloc(FLASH128_SIZE);
            memset(mem->backup, 0xFF, FLASH128_SIZE);
            mem->backup_size = FLASH128_SIZE;
            break;
        case FLASH64K:
            mem->backup = malloc(FLASH64_SIZE);
            memset(mem->backup, 0xFF, FLASH64_SIZE);
            mem->backup_size = FLASH64_SIZE;
            break;
        default:
            logfatal("Called Flash function with incorrect backup type!")
    }
}

bool atmel = false; // TODO: figure this out for real

void complete_flash_command(gbamem_t* mem, gbabus_t* bus) {
    switch (mem->flash_command) {
        case FLASHC_CHIP_ID:
#ifdef FLASH_VERBOSE_LOG
            printf("FLASHC_CHIP_ID\n");
#endif
            mem->flash_state = FLASH_CHIP_ID;
            break;
        case FLASHC_EXIT_CHIP_ID:
#ifdef FLASH_VERBOSE_LOG
            printf("FLASHC_EXIT_CHIP_ID\n");
#endif
            mem->flash_state = FLASH_READY;
            break;
        case FLASHC_BANKSWITCH:
#ifdef FLASH_VERBOSE_LOG
            printf("FLASHC_BANKSWITCH\n");
#endif
            mem->flash_state = FLASH_BANKSWITCH;
            break;
        case FLASHC_ERASE:
#ifdef FLASH_VERBOSE_LOG
            printf("FLASHC_ERASE\n");
#endif
            mem->flash_state = FLASH_ERASE;
            break;
        case FLASHC_WRITE_DATA:
            if (atmel) {
#ifdef FLASH_VERBOSE_LOG
                printf("FLASH_ERASE_WRITE_SECTOR\n");
#endif
                mem->flash_state = FLASH_ERASE_WRITE_SECTOR;
            } else {
#ifdef FLASH_VERBOSE_LOG
                printf("FLASH_WRITE_SINGLE_BYTE\n");
#endif
                mem->flash_state = FLASH_WRITE_SINGLE_BYTE;
            }
            break;
        case FLASHC_ERASE_ENTIRE_CHIP:
            logfatal("FLASHC_ERASE_ENTIRE_CHIP should be handled by the state machine below, as it is a subcommand.")
        default:
            logfatal("Unimplemented flash command: 0x%06X", mem->flash_command)
    }
}

void write_byte_flash(gbamem_t* mem, gbabus_t* bus, word address, byte value) {
#ifdef FLASH_VERBOSE_LOG
    printf("%d [0x%08X] = 0x%02X\n", mem->flash_state, address, value);
#endif
    switch (mem->flash_state) {
        case FLASH_READY:
        case FLASH_CHIP_ID:
            if (address != 0xE005555) {
                logfatal("FLASH_[CMD_READY/CHIP_ID] Tried to write to address that wasn't 0xE005555! (0x%08X)", address)
            }
            if (value == 0xF0) {
                mem->flash_state = FLASH_READY;
            } else {
                mem->flash_command = value;
                mem->flash_state = FLASH_CMD_1;
            }
            break;
        case FLASH_CMD_1:
            if (address != 0xE002AAA) {
                logfatal("FLASH_CMD_1: Tried to write to address that wasn't 0xE002AAA! (0x%08X)", address)
            }
            mem->flash_command <<= 8;
            mem->flash_command |= value;
            mem->flash_state = FLASH_CMD_2;
            break;
        case FLASH_CMD_2:
            if (address != 0xE005555) {
                logfatal("FLASH_CMD_2: Tried to write to address that wasn't 0xE005555! (0x%08X)", address)
            }
            mem->flash_command <<= 8;
            mem->flash_command |= value;
            complete_flash_command(mem, bus);
            break;
        case FLASH_ERASE:
            if (address != 0xE005555) {
                logfatal("FLASH_ERASE Tried to write to address that wasn't 0xE005555! (0x%08X)", address)
            }
            mem->flash_command = value;
            mem->flash_state = FLASH_ERASE_1;
            break;
        case FLASH_ERASE_1:
            if (address != 0xE002AAA) {
                logfatal("FLASH_ERASE_1: Tried to write to address that wasn't 0xE002AAA! (0x%08X)", address)
            }
            mem->flash_command <<= 8;
            mem->flash_command |= value;
            mem->flash_state = FLASH_ERASE_2;
            break;
        case FLASH_ERASE_2:
            mem->flash_command <<= 8;
            mem->flash_command |= value;

            if (address == 0xE005555 && mem->flash_command == FLASHC_ERASE_ENTIRE_CHIP) {
#ifdef FLASH_VERBOSE_LOG
                printf("FLASHC_ERASE_ENTIRE_CHIP\n");
#endif
                memset(mem->backup, 0xFF, mem->backup_size);
                bus->ime_temp.raw = bus->interrupt_master_enable.raw;
                bus->interrupt_master_enable.enable = false;
                mem->backup_dirty = true;
            } else {
#ifdef FLASH_VERBOSE_LOG
                printf("FLASHC_ERASE_BLOCK\n");
#endif
                word start = address & 0x000F000;
                // TODO: Replace with memset?
                for (word block = 0x000; block < 0x1000; block++) {
                    word erase_address = start | block;
                    word index = erase_address + (bus->backup_type == FLASH128K ? mem->flash_bank * 0xFFFF : 0);
                    unimplemented(index >= mem->backup_size, "Out of bounds access to backup!")
                    mem->backup[index] = 0xFF;
                }
                mem->backup_dirty = true;
            }
            // The games are told to wait until a given value == 0xFF
            // Since we're erasing by overwriting with 0xFFs, we can just go back into FLASH_READY mode
#ifdef FLASH_VERBOSE_LOG
            printf("FLASH_READY\n");
#endif
            mem->flash_state = FLASH_READY;
            break;
        case FLASH_ERASE_WRITE_SECTOR: {
            if (mem->flash_erase_write_sector_first_address == 0) {
                mem->flash_erase_write_sector_first_address = address;
            }
            word index = address & 0xFFFF;
            if (bus->backup_type == FLASH128K && mem->flash_bank == 1) {
                index += 0xFFFF;
            }
            unimplemented(index >= mem->backup_size, "Out of bounds access to backup!")
            mem->backup[index] = value;
            if ((address - 0x7F) == mem->flash_erase_write_sector_first_address) {
                bus->interrupt_master_enable.raw = bus->ime_temp.raw;
                mem->flash_erase_write_sector_first_address = 0;
                mem->flash_state = FLASH_READY;
            }
            mem->backup_dirty = true;
            break;
        }
        case FLASH_WRITE_SINGLE_BYTE: {
            word index = address & 0xFFFF;
            if (bus->backup_type == FLASH128K && mem->flash_bank == 1) {
                index += 0xFFFF;
            }
            mem->backup[index] = value;
            mem->flash_state = FLASH_READY;
            mem->backup_dirty = true;
            break;
        }
        case FLASH_BANKSWITCH:
            if (address == 0xE000000) {
                unimplemented(value > 1, "Tried to switch flash bank to nonexistent bank (>1)")
                mem->flash_bank = value;
                mem->flash_state = FLASH_READY;
            } else {
                logfatal("Unimplemented: BANK_SWITCH: [0x%08X] = 0x%02X", address, value)
            }
            break;
    }
}

byte read_byte_flash(gbamem_t* mem, word address, backup_type_t type) {
#ifdef FLASH_VERBOSE_LOG
    printf("X = [0x%08X]\n", address);
#endif
    switch (mem->flash_state) {
        case FLASH_CMD_1:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_CMD_1", address)
            break;
        case FLASH_CMD_2:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_CMD_2", address)
            break;
        case FLASH_CHIP_ID:
            if (address == 0x0E000000) {
                return MANUFACTURER; // Stubbing flash
            } else if (address == 0x0E000001) {
                return DEVICE; // Stubbing flash
            }
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_CHIP_ID", address)
            break;
        case FLASH_ERASE:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_ERASE", address)
        case FLASH_ERASE_1:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_ERASE_1", address)
        case FLASH_ERASE_2:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_ERASE_2", address)
        case FLASH_WRITE_SINGLE_BYTE:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_WRITE_SINGLE_BYTE", address)
        case FLASH_BANKSWITCH:
            logfatal("Unimplemented: Reading byte from [0x%08X] when in mode FLASH_BANK_SWITCH", address)
        case FLASH_READY:
        default: {
            word index = address & 0xFFFF;
            if (type == FLASH128K && mem->flash_bank == 1) {
                index += 0xFFFF;
            }
            unimplemented(index >= mem->backup_size, "Out of bounds access to backup!")
            return mem->backup[index];
        }
    }
    logfatal("Reached end of read_byte_flash")
}
