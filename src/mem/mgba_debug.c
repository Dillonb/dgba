#include "mgba_debug.h"
#include "../common/log.h"

#ifdef ENABLE_MGBA_DEBUG
#include <stdio.h>
#include <string.h>

#define REG_DEBUG_ENABLE 0x4FFF780
#define REG_DEBUG_FLAGS 0x4FFF700
#define REG_DEBUG_STRING 0x4FFF600

#define MGBA_LOG_FATAL 0
#define MGBA_LOG_ERROR 1
#define MGBA_LOG_WARN  2
#define MGBA_LOG_INFO  3
#define MGBA_LOG_DEBUG 4
#define MGBA_PRINT_LOG 0x100

bool mgba_debug_enabled = false;
half mgba_debug_flags = 0x0000;
char mgba_string_buffer[0x101] = "";
int mgba_buf_len = 0;

void mgba_debug_write_half(word address, half value) {
    //printf("Writing 0x%04X to 0x%08X\n", value, address);
    if (address == REG_DEBUG_ENABLE && value == 0xC0DE) {
        mgba_debug_enabled = true;
    } else if (mgba_debug_enabled && address == REG_DEBUG_FLAGS) {
        mgba_debug_flags = value;
        if (mgba_debug_flags & MGBA_PRINT_LOG) {
            mgba_debug_flags &= ~MGBA_PRINT_LOG;
            printf("[MGBA LOG] %s\n", mgba_string_buffer);
            memset(&mgba_string_buffer, 0, 0x101);
        }
    } else {
        logfatal("Writing 0x%04X to 0x%08X", value, address)
    }
}
half mgba_debug_read_half(word address) {
    if (mgba_debug_enabled && address == REG_DEBUG_ENABLE) {
        return 0x1DEA;
    }
    return 0; // TODO open bus
}

bool is_mgba_debug_enabled() {
    return mgba_debug_enabled;
}

void mgba_debug_write_byte(word address, byte value) {
    if (address >= 0x4FFF600 && address < 0x4FFF700) {
        word index = address - 0x4FFF600;
        if (index > mgba_buf_len) {
            mgba_buf_len = index;
        }
        mgba_string_buffer[index] = value;
    } else {
        logfatal("Writing 0x%02X to 0x%08X", value, address)
    }
}

#endif //ENABLE_MGBA_DEBUG
