#include <time.h>
#include "rtc.h"
#include "../../gba_system.h"

gpio_port_t gba_rtc_read() {
    return bus->port;
}

byte reverse(byte b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

#define RESET_REG    0
#define DATETIME_REG 2
#define CONTROL_REG  4
#define TIME_REG     6

byte to_bcd(byte value) {
    unimplemented(value > 99, "Can't convert a value > 99 to BCD")

    byte lsd = value % 10;
    byte msd = value / 10;

    return lsd | (msd << 4);
}

void read_register(rtc_t* rtc) {
    switch (rtc->reg) {
        case RESET_REG:
            printf("RESET\n");
            rtc->buffer = 0;
            rtc->control_reg.raw = 0;
            break;
        case DATETIME_REG: {
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);

            rtc->buffer = to_bcd(timeinfo->tm_sec);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_min);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_hour);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_wday);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_mday);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_mon + 1);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_year - 100);
            printf("DATETIME_REG: 0x%012lX\n", rtc->buffer);
            break;
        }
        case CONTROL_REG:
            rtc->buffer = rtc->control_reg.raw;
            rtc->control_reg.poweroff = 0;
            printf("CONTROL_REG: 0x%02lX\n", rtc->buffer);
            break;
        case TIME_REG: {
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);

            rtc->buffer = to_bcd(timeinfo->tm_sec);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_min);
            rtc->buffer <<= 8;

            rtc->buffer |= to_bcd(timeinfo->tm_hour);

            printf("TIME_REG: 0x%lX\n", rtc->buffer);
            break;
        }
        default:
            logfatal("Reading unknown register: 0x%X", rtc->reg)
    }

}

void write_register(rtc_t* rtc) {
    rtc->buffer = 0;
    switch (rtc->reg) {
        case RESET_REG:
            logdebug("[GPIO] We're gonna write to RESET")
            rtc->write_mask = 0x0;
            break;
        case CONTROL_REG:
            logdebug("[GPIO] We're gonna write to STATUS_REGISTER_2")
            rtc->write_mask = 0xFF;
            break;
        default:
            logfatal("Writing unknown register: 0x%X", rtc->reg)
    }
}

void run_command(byte command) {
    rtc_t* rtc = &bus->rtc;
    bool read = command >> 3;
    byte reg = command & 0x7;

    logdebug("[GPIO] [RTC] %sing register: 0x%X", read ? "Read" : "Writ", reg);

    rtc->reg = reg;

    if (read) {
        read_register(rtc);
        rtc->state = RTC_DATA_READ;
    } else {
        write_register(rtc);
        rtc->state = RTC_DATA_WRITE;
    }
}

void gba_rtc_write(gpio_port_t value, byte mask) {
    rtc_t* rtc = &bus->rtc;
    loginfo("state: %d SCK: %d SIO: %d CS: %d", rtc->state, value.rtc_sck, value.rtc_sio, value.rtc_cs);

    switch (rtc->state) {
        case RTC_READY:
            rtc->command_buffer = 0;
            rtc->current_command_bit = 0;
            if (value.rtc_cs == 0 && value.rtc_sck == 1) {
                rtc->state = RTC_COMMAND_MODE_1;
            } else {
                logfatal("Unhandled CS: %d SCK: %d combination in READY state", value.rtc_cs, value.rtc_sck)
            }
            break;
        case RTC_COMMAND_MODE_1:
            if (value.rtc_cs == 1 && value.rtc_sck == 1) {
                rtc->state = RTC_COMMAND_MODE_2;
            } else {
                logwarn("Unhandled CS: %d SCK: %d combination in RTC_COMMAND_MODE_1 state", value.rtc_cs, value.rtc_sck)
                rtc->state = RTC_COMMAND_MODE_1;
            }
            break;
        case RTC_COMMAND_MODE_2:
            if (value.rtc_cs == 1 && value.rtc_sck == 0) {
                byte bit = value.rtc_sio & 1;
                rtc->command_buffer &= ~(bit << rtc->current_command_bit);
                rtc->command_buffer |= (bit << rtc->current_command_bit);
            } else if (value.rtc_cs == 1 && value.rtc_sck == 1) {
                byte bit = value.rtc_sio & 1;
                rtc->command_buffer &= ~(bit << rtc->current_command_bit);
                rtc->command_buffer |= (bit << rtc->current_command_bit);
                if (++rtc->current_command_bit == 8) {
                    if ((rtc->command_buffer >> 4) == 6) {
                        rtc->command_buffer = reverse(rtc->command_buffer);
                        logdebug("[GPIO] [RTC] Need to reverse the command.");
                    } else if ((rtc->command_buffer & 0xF) == 6) {
                        logdebug("[GPIO] [RTC] Already got the command in a good order.");
                    }
                    // Command passed validity check and is now in the correct order. Command is in the upper 4 bits,
                    // Lower 4 are always a 6.
                    run_command((rtc->command_buffer >> 4) & 0xF);
                }
            } else {
                logfatal("Unhandled CS: %d SCK: %d combination in RTC_COMMAND_MODE_2 state (command 0x%02X)", value.rtc_cs, value.rtc_sck, rtc->command_buffer)
            }
            break;
        case RTC_DATA_READ:
            if (value.rtc_cs == 1 && value.rtc_sck == 1) {
                // If the value here used to be 0, send the next data bit
                if (bus->port.rtc_sck == 0) {
                    bus->port.rtc_sio = rtc->buffer & 1;
                    rtc->buffer >>= 1;
                }
            } else if (value.rtc_cs == 1 && value.rtc_sck == 0) {
                // Ignore
            } else if (value.rtc_cs == 0) {
                rtc->state = RTC_READY;
            } else {
                logfatal("Unhandled CS: %d SCK: %d combination in RTC_DATA_READ", value.rtc_cs, value.rtc_sck)
            }
            break;
        case RTC_DATA_WRITE:
            if (value.rtc_cs == 0 && value.rtc_sck == 1) {
                switch (rtc->reg) {
                    case RESET_REG:
                        printf("RESET\n");
                        rtc->buffer = 0;
                        rtc->control_reg.raw = 0;
                        break;
                    case CONTROL_REG:
                        rtc->control_reg.raw = reverse(rtc->buffer & 0xFF);
                        printf("Control reg is now: 0x%02X\n", rtc->control_reg.raw);
                        break;
                    default:
                        logfatal("Wrote to unknown register: %d", rtc->reg)
                }
                rtc->state = RTC_READY;
            } else if (value.rtc_cs == 1 && value.rtc_sck == 1) {
                rtc->buffer <<= 1;
                rtc->buffer &= rtc->write_mask;
            } else if (value.rtc_cs == 1 && value.rtc_sck == 0) {
                rtc->buffer &= 0xFFFFFFFFFFFFFFFE;
                rtc->buffer |= (value.rtc_sio & 1);
                printf("RTC write buffer: 0x%lX\n", rtc->buffer);
            } else {
                logfatal("Unhandled CS: %d SCK: %d combination in RTC_DATA_WRITE", value.rtc_cs, value.rtc_sck)
            }
            break;
        default:
            logfatal("Write to GPIO while in unknown state: %d", rtc->state)
    }

    bus->port.raw &= ~mask;
    bus->port.raw |= value.raw;
}
