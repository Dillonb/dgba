#include <stdio.h>

#include "gpio.h"
#include "../../common/log.h"
#include "../../gba_system.h"
#include "rtc.h"

half gpio_read(word address) {
    logdebug("gpio read from address 0x%08X", address);
    if (bus->allow_gpio_read) {
        switch (address & 0xF) {
            case 0x4: // Data
                switch (bus->gpio_device) {
                    case RTC: {
                        gpio_port_t port = gba_rtc_read();
                        return bus->gpio_read_mask & port.raw;
                    }
                    default:
                        logfatal("Read from data port for unimplemented device %d", bus->gpio_device)
                }
                break;
            case 0x6:
                return bus->gpio_write_mask;
            case 0x8:
                return bus->allow_gpio_read & 1;
        }
    }
    return 0;
}

void gpio_write(word address, half value) {
    logdebug("gpio write 0x%04X to address 0x%08X", value, address);
    switch (address & 0xF) {
        case 0x4: // Data
            switch (bus->gpio_device) {
                case RTC: {
                    gpio_port_t port;
                    port.raw = value & bus->gpio_write_mask;
                    gba_rtc_write(port, bus->gpio_write_mask);
                    break;
                }
                default:
                    logfatal("Write to data port for unimplemented device %d", bus->gpio_device)
            }
            break;
        case 0x6:
            bus->gpio_write_mask = value    & 0b1111;
            bus->gpio_read_mask  = (~value) & 0b1111;
            logdebug("Set the read mask to 0x%X and the write mask to 0x%X", bus->gpio_read_mask, bus->gpio_write_mask);
            break;
        case 0x8:
            bus->allow_gpio_read = (value & 1) == 1;
            if (bus->allow_gpio_read) {
                logdebug("GPIO is now READABLE!");
            } else {
                logdebug("GPIO is now UNREADABLE!");
            }
            break;
    }
}
