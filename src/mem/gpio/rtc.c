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

#define DATETIME_REG 2
#define STATUS_REG_2 4

byte to_bcd(byte value) {
    unimplemented(value > 99, "Can't convert a value > 99 to BCD")

    byte lsd = value % 10;
    byte msd = value / 10;

    return lsd | (msd << 4);
}

void run_command(byte command) {
    rtc_t* rtc = &bus->rtc;
    bool read = command >> 3;
    byte reg = command & 0x7;

    logdebug("%sing register: 0x%X", read ? "Read" : "Write", reg);

    switch (reg) {
        case DATETIME_REG: {
            time_t rawtime;
            struct tm * timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);

            rtc->read_buffer = to_bcd(timeinfo->tm_sec);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_min);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_hour);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_wday);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_mday);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_mon + 1);
            rtc->read_buffer <<= 8;

            rtc->read_buffer |= to_bcd(timeinfo->tm_year - 100);
            break;
        }
        case STATUS_REG_2:
            rtc->read_buffer = 0; // TODO
            rtc->state = RTC_READY;
            break;
        default:
            logfatal("%sing unknown register: 0x%X", read ? "Read" : "Writ", reg)
    }

    if (read) {
        rtc->state = RTC_DATA_READ;
    } else {
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
                        logdebug("Need to reverse the command.");
                    } else if ((rtc->command_buffer & 0xF) == 6) {
                        logdebug("Already got the command in a good order.");
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
                    bus->port.rtc_sio = rtc->read_buffer & 1;
                    rtc->read_buffer >>= 1;
                }
            } else if (value.rtc_cs == 1 && value.rtc_sck == 0) {
                // Ignore
            } else if (value.rtc_cs == 0) {
                rtc->state = RTC_READY;
            } else {
                logfatal("Unhandled CS: %d SCK: %d combination in RTC_DATA_READ", value.rtc_cs, value.rtc_sck)
            }
            break;
        default:
            logfatal("Write to GPIO while in unknown state: %d", rtc->state)
    }

    bus->port.raw &= ~mask;
    bus->port.raw |= value.raw;
}
