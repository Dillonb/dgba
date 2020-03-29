#include <stdbool.h>
#include <capstone/capstone.h>

#include "disassemble.h"
#include "common/log.h"

bool disassembler_initialized = false;
csh handle_thumb;
csh handle_arm;
cs_insn* insn;

void disassembler_initialize() {
    if (disassembler_initialized) {
        return;
    }

    if (cs_open(CS_ARCH_ARM, CS_MODE_THUMB | CS_MODE_LITTLE_ENDIAN, &handle_thumb) != CS_ERR_OK) {
        logfatal("Failed to initialize capstone for ARM THUMB")
    }

    if (cs_open(CS_ARCH_ARM, CS_MODE_LITTLE_ENDIAN, &handle_arm) != CS_ERR_OK) {
        logfatal("Failed to initialize capstone for ARM")
    }

    disassembler_initialized = true;
}

int disassemble_thumb(word address, half raw_thumb, char* buf, int buflen) {
    byte code[2];
    code[0] = raw_thumb & 0xFF;
    code[1] = (raw_thumb >> 8) & 0xFF;
    disassembler_initialize();
    size_t count = cs_disasm(handle_thumb, code, 2, address, 0, &insn);
    if (count == 0) {
        logwarn("Failed to disassemble code!")
        snprintf(buf, buflen, "ERROR! (0x%04X) little 0x%04X", raw_thumb, FAKELITTLE_HALF(raw_thumb));
        return 0;
    } else if (count > 1) {
        logwarn("Given more than one instruction, only disassembling the first one!")
    }

    snprintf(buf, buflen, "%s %s", insn[0].mnemonic, insn[0].op_str);

    cs_free(insn, count);

    return 1;
}

int disassemble_arm(word address, word raw_arm, char* buf, int buflen) {
    byte code[4];
    code[0] = raw_arm & 0xFF;
    code[1] = (raw_arm >> 8) & 0xFF;
    code[2] = (raw_arm >> 16) & 0xFF;
    code[3] = (raw_arm >> 24) & 0xFF;
    disassembler_initialize();
    size_t count = cs_disasm(handle_arm, code, 4, address, 0, &insn);
    if (count == 0) {
        logwarn("Failed to disassemble code!")
        snprintf(buf, buflen, "ERROR! big (0x%08X) little (0x%08X)", raw_arm, FAKELITTLE_WORD(raw_arm));
        return 0;
    } else if (count > 1) {
        logwarn("Given more than one instruction, only disassembling the first one!")
    }

    snprintf(buf, buflen, "%s %s", insn[0].mnemonic, insn[0].op_str);

    cs_free(insn, count);

    return 1;
}
