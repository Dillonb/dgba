#ifndef GBA_SIGN_EXTENSION_H
#define GBA_SIGN_EXTENSION_H

#include "../common/util.h"

uint64_t sign_extend_64(uint64_t v, int old, int new);
word sign_extend_word(word v, int old, int new);

#endif //GBA_SIGN_EXTENSION_H
