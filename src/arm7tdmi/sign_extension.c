#include <stdbool.h>
#include "sign_extension.h"
#include "../common/log.h"

uint64_t sign_extend_64(uint64_t v, int old, int new) {
    unimplemented(new < old, "Can't downsize signed values with this function!")

    uint64_t mask = v & (1 << (old - 1));
    bool s = mask > 0;
    if (s) {
        return (v ^ mask) - mask;
    }
    else {
        return v; // No sign bit set, don't need to sign extend
    }
}

word sign_extend_word(word v, int old, int new) {
    unimplemented(new < old, "Can't downsize signed values with this function!")

    word mask = v & (1 << (old - 1));
    bool s = mask > 0;
    if (s) {
        return (v ^ mask) - mask;
    }
    else {
        return v; // No sign bit set, don't need to sign extend
    }
}
