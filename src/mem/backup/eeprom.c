#include <string.h>
#include "eeprom.h"

#define EEPROM8K_SIZE 0x2000

#define EEPROM_COMMAND_READ 0b11
#define EEPROM_COMMAND_WRITE 0b10

// How many times should the game be required to read from EEPROM after a write until it is considered ready?
#define EEPROM_READS_UNTIL_READY 10

void init_eeprom(gbamem_t* mem, eeprom_size_t size) {
    size_t malloc_size;
    switch (size) {
        case EEPROM_512:
            logfatal("EEPROM 512 unimplemented")
            //malloc_size = EEPROM512_SIZE;
            break;
        case EEPROM_8K:
            malloc_size = EEPROM8K_SIZE;
            break;
    }
    mem->backup = malloc(malloc_size);
    memset(mem->backup, 0xFF, malloc_size);
    mem->backup_size = malloc_size;
    mem->eeprom_initialized = true;
    read_persisted_backup();
}

void handle_eeprom_command(gbamem_t* mem, byte command) {
    switch (command) {
        case EEPROM_COMMAND_READ:
            mem->eeprom_state = EEPROM_ACCEPT_READ_ADDRESS;
            mem->eeprom_address = 0;
            mem->eeprom_bits_remaining = mem->backup_size == EEPROM8K_SIZE ? 14 : 6;
            break;
        case EEPROM_COMMAND_WRITE:
            mem->eeprom_state = EEPROM_ACCEPT_WRITE_ADDRESS;
            mem->eeprom_address = 0;
            mem->eeprom_bits_remaining = mem->backup_size == EEPROM8K_SIZE ? 14 : 6;
            break;
        default:
            logfatal("Unknown EEPROM command: %d", command);
    }
}

half read_half_eeprom(gbabus_t* bus, gbamem_t* mem, word address) {
    if (!mem->eeprom_initialized) {
        logfatal("Read from EEPROM before initialized!")
    }
    switch (mem->eeprom_state) {
        case EEPROM_READ:
            // Return 4 garbage bits
            if (mem->eeprom_bits_remaining > 64) {
                mem->eeprom_bits_remaining--;
                logwarn("Returning %d/4 garbage bits", 5 - (mem->eeprom_bits_remaining - 63))
                return 0x0000;
            } else if (mem->eeprom_bits_remaining >= 0) {
                int bit = 64 - mem->eeprom_bits_remaining;
                if (--mem->eeprom_bits_remaining == 0) {
                    mem->eeprom_state = EEPROM_READY; // Entire stream has been read
                }
                int byte_offset = bit / 8;
                int bit_in_byte = 7 - (bit % 8); // MSB first
                logwarn("Transferring %d/64 data bits (byte offset 0x%02X from base address, bit %d)", bit + 1, byte_offset, bit_in_byte)
                word index = mem->eeprom_address + byte_offset;
                if (index > mem->backup_size) {
                    logfatal("Access to EEPROM outside backup space! Index: %d Backup size: %zu", index, mem->backup_size)
                }
                byte value = mem->backup[index];
                return (value >> bit_in_byte) & 1;
            } else {
                logfatal("Tried to read from EEPROM when no data was remaining")
            }
        case EEPROM_POST_WRITE:
            if (--mem->eeprom_bits_remaining <= 0) {
                mem->backup_dirty = true;
                mem->eeprom_state = EEPROM_READY;
                return 0x0001;
            } else {
                return 0x0000;
            }
        default:
            logfatal("Read from EEPROM in unknown state: %d", mem->eeprom_state)
    }
}

void write_half_eeprom(int active_dma, gbabus_t* bus, gbamem_t* mem, word address, half value) {
    if (!mem->eeprom_initialized) {
        int wc;
        switch (active_dma) {
            case 0:
                wc = bus->DMA0CNT_L.wc;
                break;
            case 1:
                wc = bus->DMA1CNT_L.wc;
                break;
            case 2:
                wc = bus->DMA2CNT_L.wc;
                break;
            case 3:
                wc = bus->DMA3CNT_L.wc;
                break;
        }

        switch (wc) {
            case 9: // 9 bits = 2 for command, 6 for address, 1 to end command
                logfatal("EEPROM_512 unimplemented")
            case 17: // 17 bits = 2 for command, 14 for address, 1 to end command
                if (!mem->eeprom_initialized) {
                    init_eeprom(mem, EEPROM_8K);
                }
                break;
            default:
                logfatal("Write to EEPROM with active DMA %d and a WC of %d", active_dma, wc)
        }
    }

    switch (mem->eeprom_state) {
        case EEPROM_READY:
            mem->eeprom_command = (value & 1) << 1;
            mem->eeprom_state = EEPROM_CMD_1;
            break;
        case EEPROM_CMD_1:
            mem->eeprom_command |= (value & 1);
            handle_eeprom_command(mem, mem->eeprom_command);
            break;
        case EEPROM_ACCEPT_READ_ADDRESS:
            if (mem->eeprom_bits_remaining > 0) {
                mem->eeprom_address <<= 1;
                mem->eeprom_address |= (value & 1);
                mem->eeprom_bits_remaining--;
            } else {
                if ((value & 1) != 0) {
                    logwarn("Expected a value with an LSB of 0 here, got %d (0x%04X)", value & 1, value)
                }
                logwarn("Reading 64 bits from EEPROM starting at address: 0x%04X", mem->eeprom_address)
                mem->eeprom_state = EEPROM_READ;
                mem->eeprom_bits_remaining = 64 + 4; // Plus 4 garbage bits
            }
            break;
        case EEPROM_ACCEPT_WRITE_ADDRESS:
            if (mem->eeprom_bits_remaining > 0) {
                mem->eeprom_address <<= 1;
                mem->eeprom_address |= (value & 1);
                if (--mem->eeprom_bits_remaining == 0) {
                    logwarn("Writing 64 bits to EEPROM starting at address: 0x%04X", mem->eeprom_address)
                    mem->eeprom_state = EEPROM_WRITE;
                    mem->eeprom_bits_remaining = 64;
                }
            } else {
                logfatal("Trying to accept a write address when one is unexpected!")
            }
            break;
        case EEPROM_WRITE: {
            int bit = 64 - mem->eeprom_bits_remaining;
            if (mem->eeprom_bits_remaining > 0) {
                int byte_offset = bit / 8;
                int bit_in_byte = 7 - (bit % 8); // MSB first
                byte mask = ~(1 << bit_in_byte);
                logwarn("Transferring %d/64 data bits (byte offset 0x%02X from base address, bit %d)", bit + 1,
                        byte_offset, bit_in_byte)
                byte eeprom_value = mem->backup[mem->eeprom_address + byte_offset];
                byte old_value = eeprom_value;
                eeprom_value &= mask;
                eeprom_value |= ((value & 1) << bit_in_byte);
                word index = mem->eeprom_address + byte_offset;
                if (index > mem->backup_size) {
                    logfatal("Access to EEPROM outside backup space! Index: %d Backup size: %zu", index, mem->backup_size)
                }
                mem->backup[index] = eeprom_value;
                mem->backup_dirty = true;
                logwarn("Wrote bit %d to bit number %d in byte offset: %d. Old value: 0x%02X New value: 0x%02X",
                        value & 1, bit_in_byte, byte_offset, old_value, eeprom_value);
            } else if (mem->eeprom_bits_remaining == 0) {
                if ((value & 1) != 0) {
                    logfatal("Expected a write with an LSB of 0 after an EEPROM write stream, instead, got: 0x%04X",
                             value)
                }
                mem->eeprom_bits_remaining = EEPROM_READS_UNTIL_READY;
                mem->eeprom_state = EEPROM_POST_WRITE; // Entire stream has been written
                logwarn("EEPROM write stream completed.")
            } else {
                logfatal("Tried to read from EEPROM when no data was remaining")
            }
            mem->eeprom_bits_remaining--;
            break;
        }
        default:
            logfatal("Write half 0x%04X to EEPROM in unknown state: %d at addr 0x%08X", value, mem->eeprom_state, address)
    }
}
