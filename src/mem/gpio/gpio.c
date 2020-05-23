#include <stdio.h>

#include "gpio.h"
#include "../../common/log.h"
#include "../../gba_system.h"

half gpio_read(word address) {
    printf("gpio read from address 0x%08X\n", address);
    if (bus->allow_gpio_read) {
        switch (address & 0xF) {
            case 0x4: // Data
                logfatal("Read from data port unimplemented")
                break;
            case 0x6:
                return bus->gpio_port_direction & 0b1111;
            case 0x8:
                return bus->allow_gpio_read & 1;
        }
    }
    return 0;
}

void gpio_write(word address, half value) {
    printf("gpio write 0x%04X to address 0x%08X\n", value, address);
    switch (address & 0xF) {
        case 0x4: // Data
            //logfatal("Write to data port unimplemented")
            break;
        case 0x6:
            bus->gpio_port_direction = value & 0b1111;
            break;
        case 0x8:
            bus->allow_gpio_read = (value & 1) == 1;
            break;
    }
}
