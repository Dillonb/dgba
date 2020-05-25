#ifndef GBA_GPIO_H
#define GBA_GPIO_H

#include "../../common/util.h"

half gpio_read(word address);
void gpio_write(word address, half value);

#endif //GBA_GPIO_H
