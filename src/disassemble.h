#ifndef GBA_DISASSEMBLE_H
#define GBA_DISASSEMBLE_H

#include "common/util.h"

int disassemble_thumb(word address, half raw_thumb, char* buf, int buflen);
int disassemble_arm(word address, word raw_arm, char* buf, int buflen);

#endif //GBA_DISASSEMBLE_H
