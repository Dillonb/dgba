#include <string.h>

#include "gbabus.h"
#include "ioreg_util.h"
#include "ioreg_names.h"
#include "gbabios.h"
#include "../gba_system.h"
#include "backup/flash.h"
#include "gpio/gpio.h"


INLINE word open_bus(word addr);

#define REGION_BIOS       0x00
#define REGION_EWRAM      0x02
#define REGION_IWRAM      0x03
#define REGION_IOREG      0x04
#define REGION_PRAM       0x05
#define REGION_VRAM       0x06
#define REGION_OAM        0x07
#define REGION_GAMEPAK0_L 0x08
#define REGION_GAMEPAK0_H 0x09
#define REGION_GAMEPAK1_L 0x0A
#define REGION_GAMEPAK1_H 0x0B
#define REGION_GAMEPAK2_L 0x0C
#define REGION_GAMEPAK2_H 0x0D
#define REGION_SRAM       0x0E
#define REGION_SRAM_MIRR  0x0F

const int byte_half_cycles[] = {
        [REGION_BIOS]       = 1,
        [REGION_EWRAM]      = 3,
        [REGION_IWRAM]      = 1,
        [REGION_IOREG]      = 1,
        [REGION_PRAM]       = 1,
        [REGION_VRAM]       = 1,
        [REGION_OAM]        = 1,
        [REGION_GAMEPAK0_L] = 5,
        [REGION_GAMEPAK0_H] = 5,
        [REGION_GAMEPAK1_L] = 5,
        [REGION_GAMEPAK1_H] = 5,
        [REGION_GAMEPAK2_L] = 5,
        [REGION_GAMEPAK2_H] = 5,
        [REGION_SRAM]       = 5,
        [REGION_SRAM_MIRR]  = 5
};

const int word_cycles[] = {
        [REGION_BIOS]       = 1,
        [REGION_EWRAM]      = 6,
        [REGION_IWRAM]      = 1,
        [REGION_IOREG]      = 1,
        [REGION_PRAM]       = 1,
        [REGION_VRAM]       = 1,
        [REGION_OAM]        = 1,
        [REGION_GAMEPAK0_L] = 8,
        [REGION_GAMEPAK0_H] = 8,
        [REGION_GAMEPAK1_L] = 8,
        [REGION_GAMEPAK1_H] = 8,
        [REGION_GAMEPAK2_L] = 8,
        [REGION_GAMEPAK2_H] = 8,
        [REGION_SRAM]       = 8,
        [REGION_SRAM_MIRR]  = 8
};

gbabus_t* init_gbabus() {
    gbabus_t* bus_state = malloc(sizeof(gbabus_t));
    memset(bus_state, 0, sizeof(gbabus_t));
    bus_state->interrupt_master_enable.raw = 0;
    bus_state->interrupt_enable.raw = 0;
    bus_state->KEYINPUT.raw = 0xFFFF;
    bus_state->SOUNDBIAS.raw = 0x0200;

    bus_state->DMA0INT.previously_enabled = false;
    bus_state->DMA1INT.previously_enabled = false;
    bus_state->DMA2INT.previously_enabled = false;
    bus_state->DMA3INT.previously_enabled = false;

    // Loop through every single word aligned address in the ROM and try to find what backup type it is (you read that right)
    // Start at 0xE4 since everything before that is part of the header
    for (int addr = 0xE4; addr < (mem->rom_size - 9); addr += 4) {
        if (memcmp("SRAM", &mem->rom[addr], 4) == 0) {
            bus_state->backup_type = SRAM;
            logwarn("Determined backup type: SRAM")
            mem->backup = malloc(SRAM_SIZE);
            mem->backup_size = SRAM_SIZE;
            memset(mem->backup, 0, SRAM_SIZE);
            break;
        }
        if (memcmp("EEPROM", &mem->rom[addr], 6) == 0) {
            bus_state->backup_type = EEPROM;
            logfatal("Determined backup type: EEPROM")
            break;
        }
        if (memcmp("FLASH_", &mem->rom[addr], 6) == 0) {
            bus_state->backup_type = FLASH64K;
            init_flash(mem, FLASH64K);
            break;
        }
        if (memcmp("FLASH512_", &mem->rom[addr], 9) == 0) {
            bus_state->backup_type = FLASH64K;
            init_flash(mem, FLASH64K);
            break;
        }
        if (memcmp("FLASH1M_", &mem->rom[addr], 8) == 0) {
            bus_state->backup_type = FLASH128K;
            init_flash(mem, FLASH128K);
            break;
        }
    }

    if (bus_state->backup_type != UNKNOWN) {
        FILE *fp = fopen(mem->backup_path, "rb");
        if (fp != NULL) {
            fread(mem->backup, mem->backup_size, 1, fp);
            fclose(fp);
        }
    }

    bus_state->gpio_read_mask = 0;
    bus_state->gpio_write_mask = 0xF;

    bus_state->rtc.control_reg.raw = 0x82;
    bus_state->rtc.control_reg.mode_24h = true;

    return bus_state;
}

KEYINPUT_t* get_keyinput() {
    return &bus->KEYINPUT;
}

void request_interrupt(gba_interrupt_t interrupt) {
    if (bus->interrupt_master_enable.enable) {
        switch (interrupt) {
            case IRQ_VBLANK:
                if (bus->interrupt_enable.lcd_vblank) {
                    cpu->irq = true;
                    bus->IF.vblank = true;
                } else {
                    logwarn("VBlank interrupt blocked by IE")
                }
                break;
            case IRQ_HBLANK:
                if (bus->interrupt_enable.lcd_hblank) {
                    cpu->irq = true;
                    bus->IF.hblank = true;
                } else {
                    logwarn("HBlank interrupt blocked by IE")
                }
                break;
            case IRQ_VCOUNT:
                if (bus->interrupt_enable.lcd_vcounter_match) {
                    cpu->irq = true;
                    bus->IF.vcount = true;
                } else {
                    logwarn("VCount interrupt blocked by IE")
                }
                break;
            case IRQ_TIMER0:
                if (bus->interrupt_enable.timer0_overflow) {
                    cpu->irq = true;
                    bus->IF.timer0 = true;
                } else {
                    logwarn("Timer0 overflow interrupt blocked by IE")
                }
                break;
            case IRQ_TIMER1:
                if (bus->interrupt_enable.timer1_overflow) {
                    cpu->irq = true;
                    bus->IF.timer1 = true;
                } else {
                    logwarn("Timer1 overflow interrupt blocked by IE")
                }
                break;
            case IRQ_TIMER2:
                if (bus->interrupt_enable.timer2_overflow) {
                    cpu->irq = true;
                    bus->IF.timer2 = true;
                } else {
                    logwarn("Timer2 overflow interrupt blocked by IE")
                }
                break;
            case IRQ_TIMER3:
                if (bus->interrupt_enable.timer3_overflow) {
                    cpu->irq = true;
                    bus->IF.timer3 = true;
                } else {
                    logwarn("Timer3 overflow interrupt blocked by IE")
                }
                break;
            case IRQ_DMA0:
                if (bus->interrupt_enable.dma_0) {
                    cpu->irq = true;
                    bus->IF.dma0 = true;
                } else {
                    logwarn("DMA0 interrupt blocked by IE")
                }
                break;
            case IRQ_DMA1:
                if (bus->interrupt_enable.dma_1) {
                    cpu->irq = true;
                    bus->IF.dma1 = true;
                } else {
                    logwarn("DMA1 interrupt blocked by IE")
                }
                break;
            case IRQ_DMA2:
                if (bus->interrupt_enable.dma_2) {
                    cpu->irq = true;
                    bus->IF.dma2 = true;
                } else {
                    logwarn("DMA2 interrupt blocked by IE")
                }
                break;
            case IRQ_DMA3:
                if (bus->interrupt_enable.dma_3) {
                    cpu->irq = true;
                    bus->IF.dma3 = true;
                } else {
                    logwarn("DMA3 interrupt blocked by IE")
                }
                break;
            default:
                logfatal("Unknown interrupt index %d requested!", interrupt)
        }
    } else {
        logwarn("Interrupt blocked by IME")
    }
}

INLINE void write_half_ioreg_masked(word addr, half value, half mask);
INLINE void write_word_ioreg_masked(word addr, word value, word mask);

INLINE void write_byte_ioreg(word addr, byte value) {
    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable byte ioreg 0x%08X", addr)
        return;
    }
    byte size = get_ioreg_size_for_addr(addr);
    if (size == sizeof(half)) {
        word offset = (addr % sizeof(half));
        half shifted_value = value;
        shifted_value <<= (offset * 8);
        write_half_ioreg_masked(addr - offset, shifted_value, 0xFF << (offset * 8));
    } else if (size == sizeof(word)) {
        word offset = (addr % sizeof(word));
        word shifted_value = value;
        shifted_value <<= (offset * 8);
        write_word_ioreg_masked(addr - offset, shifted_value, 0xFF << (offset * 8));
    } else {
        // Don't really need to do the get addr/write masked thing that the bigger ioregs do
        // Or do I?
        word regnum = addr & 0xFFF;
        switch (regnum) {
            case IO_HALTCNT: {
                if ((value & 1) == 0) {
                    logwarn("HALTING CPU!")
                    cpu->halt = true;
                } else {
                    logfatal("Wrote to HALTCNT with bit 0 being 1")
                }
                break;
            }
            case IO_POSTFLG:
                logwarn("Ignoring write to POSTFLG register")
                break;
            default:
                logfatal("Write to unknown (but valid) byte ioreg addr 0x%08X == 0x%02X", addr, value)
        }
    }
}

INLINE byte read_byte_ioreg(word addr) {
    switch (addr & 0xFFF) {
        case IO_POSTFLG:
            logwarn("Ignoring read from POSTFLG reg. Returning 0")
            return 0;
        default:
            logfatal("Reading byte ioreg at 0x%08X", addr)
    }
}

INLINE half* get_half_ioreg_ptr(word addr, bool write) {
    word regnum = addr & 0xFFF;
    switch (regnum) {
        case IO_DISPCNT: return &ppu->DISPCNT.raw;
        case IO_DISPSTAT: return &ppu->DISPSTAT.raw;
        case IO_BG0CNT: return &ppu->BG0CNT.raw;
        case IO_BG1CNT: return &ppu->BG1CNT.raw;
        case IO_BG2CNT: return &ppu->BG2CNT.raw;
        case IO_BG3CNT: return &ppu->BG3CNT.raw;
        case IO_BG0HOFS: return &ppu->BG0HOFS.raw;
        case IO_BG1HOFS: return &ppu->BG1HOFS.raw;
        case IO_BG2HOFS: return &ppu->BG2HOFS.raw;
        case IO_BG3HOFS: return &ppu->BG3HOFS.raw;
        case IO_BG0VOFS: return &ppu->BG0VOFS.raw;
        case IO_BG1VOFS: return &ppu->BG1VOFS.raw;
        case IO_BG2VOFS: return &ppu->BG2VOFS.raw;
        case IO_BG3VOFS: return &ppu->BG3VOFS.raw;
        case IO_BG2PA: return &ppu->BG2PA.raw;
        case IO_BG2PB: return &ppu->BG2PB.raw;
        case IO_BG2PC: return &ppu->BG2PC.raw;
        case IO_BG2PD: return &ppu->BG2PD.raw;
        case IO_BG3PA: return &ppu->BG3PA.raw;
        case IO_BG3PB: return &ppu->BG3PB.raw;
        case IO_BG3PC: return &ppu->BG3PC.raw;
        case IO_BG3PD: return &ppu->BG3PD.raw;
        case IO_WIN0H: return &ppu->WIN0H.raw;
        case IO_WIN1H: return &ppu->WIN1H.raw;
        case IO_WIN0V: return &ppu->WIN0V.raw;
        case IO_WIN1V: return &ppu->WIN1V.raw;
        case IO_WININ: return &ppu->WININ.raw;
        case IO_WINOUT: return &ppu->WINOUT.raw;
        case IO_MOSAIC: return &ppu->MOSAIC.raw;
        case IO_BLDCNT: return &ppu->BLDCNT.raw;
        case IO_BLDALPHA: return &ppu->BLDALPHA.raw;
        case IO_BLDY: return &ppu->BLDY.raw;
        case IO_IE: return &bus->interrupt_enable.raw;
        case IO_DMA0CNT_L: return &bus->DMA0CNT_L.raw;
        case IO_DMA0CNT_H: return &bus->DMA0CNT_H.raw;
        case IO_DMA1CNT_L: return &bus->DMA1CNT_L.raw;
        case IO_DMA1CNT_H: return &bus->DMA1CNT_H.raw;
        case IO_DMA2CNT_L: return &bus->DMA2CNT_L.raw;
        case IO_DMA2CNT_H: return &bus->DMA2CNT_H.raw;
        case IO_DMA3CNT_L: return &bus->DMA3CNT_L.raw;
        case IO_DMA3CNT_H: return &bus->DMA3CNT_H.raw;
        case IO_KEYINPUT: return &bus->KEYINPUT.raw;
        case IO_RCNT: return &bus->RCNT.raw;
        case IO_JOYCNT: return &bus->JOYCNT.raw;
        case IO_IME: return &bus->interrupt_master_enable.raw;
        case IO_SOUNDBIAS: return &bus->SOUNDBIAS.raw;
        case IO_TM0CNT_L: {
            if (write) {
                return &bus->TMCNT_L[0].raw;
            } else {
                return &bus->TMINT[0].value;
            }
        }
        case IO_TM0CNT_H: return &bus->TMCNT_H[0].raw;
        case IO_TM1CNT_L: {
            if (write) {
                return &bus->TMCNT_L[1].raw;
            } else {
                return &bus->TMINT[1].value;
            }
        }
        case IO_TM1CNT_H: return &bus->TMCNT_H[1].raw;
        case IO_TM2CNT_L: {
            if (write) {
                return &bus->TMCNT_L[2].raw;
            } else {
                return &bus->TMINT[2].value;
            }
        }
        case IO_TM2CNT_H: return &bus->TMCNT_H[2].raw;
        case IO_TM3CNT_L: {
            if (write) {
                return &bus->TMCNT_L[3].raw;
            } else {
                return &bus->TMINT[3].value;
            }
        }
        case IO_TM3CNT_H: return &bus->TMCNT_H[3].raw;
        case IO_KEYCNT: return &bus->KEYCNT.raw;
        case IO_IF: return &bus->IF.raw;
        case IO_WAITCNT:
            return &bus->WAITCNT.raw;
        case IO_UNDOCUMENTED_GREEN_SWAP:
            logwarn("Ignoring access to Green Swap register")
            return NULL;
        case IO_VCOUNT: return &ppu->y;
        case IO_SOUND1CNT_L:
        case IO_SOUND1CNT_H:
        case IO_SOUND1CNT_X:
        case IO_SOUND2CNT_L:
        case IO_SOUND2CNT_H:
        case IO_SOUND3CNT_L:
        case IO_SOUND3CNT_H:
        case IO_SOUND3CNT_X:
        case IO_SOUND4CNT_L:
        case IO_SOUND4CNT_H:
        case IO_SOUNDCNT_L:
        case IO_SOUNDCNT_H:
        case IO_SOUNDCNT_X:
        case WAVE_RAM0_L:
        case WAVE_RAM0_H:
        case WAVE_RAM1_L:
        case WAVE_RAM1_H:
        case WAVE_RAM2_L:
        case WAVE_RAM2_H:
        case WAVE_RAM3_L:
        case WAVE_RAM3_H:
            logwarn("Ignoring access to sound register: 0x%03X", regnum)
            return NULL;

        case IO_SIOCNT:
        case IO_SIOMULTI0:
        case IO_SIOMULTI1:
        case IO_SIOMULTI2:
        case IO_SIOMULTI3:
        case IO_SIOMLT_SEND:
        case IO_JOY_RECV:
        case IO_JOY_TRANS:
        case IO_JOYSTAT:
            logwarn("Ignoring access to SIO register: 0x%03X", regnum)
            return NULL;

        default:
            logfatal("Access to unknown (but valid) half ioreg addr 0x%08X", addr)
    }
}

void tmcnth_write(int n, half old_value) {
    TMCNT_H_t old;
    old.raw = old_value;
    bus->TMSTART[n] = bus->TMCNT_H[n].start;
    if (!old.start && bus->TMCNT_H[n].start) {
        bus->TMINT[n].ticks = 0;
        bus->TMINT[n].value = bus->TMCNT_L[n].timer_reload;
    }

    bus->num_active_timers = 0;
    for (int i = 0; i < 4; i++) {
        if (bus->TMCNT_H[i].start && !bus->TMCNT_H[i].cascade) {
            bus->TMACTIVE[bus->num_active_timers++] = i;
        }
    }
}

INLINE void write_half_ioreg_masked(word addr, half value, half mask) {
    half* ioreg = get_half_ioreg_ptr(addr, true);
    if (ioreg) {
        switch (addr & 0xFFF) {
            case IO_DISPSTAT:
                mask &= 0b1111111111111000; // Last 3 bits are read-only
                break;
            case IO_IME:
                mask = 0b1;
                break;
            case IO_IF: {
                bus->IF.raw &= ~value;
                return;
            }
            default:
                break; // No special case
        }
        half old_value = *ioreg;
        *ioreg &= (~mask);
        *ioreg |= (value & mask);
        switch (addr & 0xFFF) {
            case IO_DMA0CNT_H:
                if (!bus->DMA0CNT_H.dma_enable) {
                    bus->DMA0INT.previously_enabled = false;
                }
                gba_dma();
                break;
            case IO_DMA1CNT_H:
                if (!bus->DMA1CNT_H.dma_enable) {
                    bus->DMA1INT.previously_enabled = false;
                }
                gba_dma();
                break;
            case IO_DMA2CNT_H:
                if (!bus->DMA2CNT_H.dma_enable) {
                    bus->DMA2INT.previously_enabled = false;
                }
                gba_dma();
                break;
            case IO_DMA3CNT_H:
                if (!bus->DMA3CNT_H.dma_enable) {
                    bus->DMA3INT.previously_enabled = false;
                }
                gba_dma();
                break;
            case IO_DMA0CNT_L:
            case IO_DMA1CNT_L:
            case IO_DMA2CNT_L:
            case IO_DMA3CNT_L:
                gba_dma();
                break;
            case IO_TM0CNT_H:
                tmcnth_write(0, old_value);
                break;
            case IO_TM1CNT_H:
                tmcnth_write(1, old_value);
                break;
            case IO_TM2CNT_H:
                tmcnth_write(2, old_value);
                break;
            case IO_TM3CNT_H:
                tmcnth_write(3, old_value);
                break;
        }
    } else {
        logwarn("Ignoring write to half ioreg 0x%08X", addr)
    }
}

INLINE void write_half_ioreg(word addr, half value) {
    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable half ioreg 0x%08X", addr)
        return;
    }
    // Write to the whole thing
    write_half_ioreg_masked(addr, value, 0xFFFF);
}

INLINE word* get_word_ioreg_ptr(word addr) {
    unimplemented(get_ioreg_size_for_addr(addr) != sizeof(word), "Trying to get the address of a wrong-sized word ioreg")
    switch (addr & 0xFFF) {
        case IO_BG2X:     return &ppu->BG2X.initial.raw;
        case IO_BG2Y:     return &ppu->BG2Y.initial.raw;
        case IO_BG3X:     return &ppu->BG3X.initial.raw;
        case IO_BG3Y:     return &ppu->BG3Y.initial.raw;
        case IO_DMA0SAD:  return &bus->DMA0SAD.raw;
        case IO_DMA0DAD:  return &bus->DMA0DAD.raw;
        case IO_DMA1SAD:  return &bus->DMA1SAD.raw;
        case IO_DMA1DAD:  return &bus->DMA1DAD.raw;
        case IO_DMA2SAD:  return &bus->DMA2SAD.raw;
        case IO_DMA2DAD:  return &bus->DMA2DAD.raw;
        case IO_DMA3SAD:  return &bus->DMA3SAD.raw;
        case IO_DMA3DAD:  return &bus->DMA3DAD.raw;
        case IO_FIFO_A:
        case IO_FIFO_B:
        case IO_JOY_RECV:
        case IO_JOY_TRANS:
        case IO_IMEM_CTRL:
            return NULL;
        default: logfatal("Tried to get the address of an unknown (but valid) word ioreg addr: 0x%08X", addr)
    }
}

INLINE void write_word_ioreg_masked(word addr, word value, word mask) {
    word maskedaddr = addr & 0xFFF;
    switch (maskedaddr) {
        case IO_FIFO_A:
            unimplemented(mask != 0xFFFFFFFF, "Write to FIFO not all at once")
            write_fifo(apu, 0, value);
            break;
        case IO_FIFO_B:
            unimplemented(mask != 0xFFFFFFFF, "Write to FIFO not all at once")
            write_fifo(apu, 1, value);
            break;
        case IO_BG2X:
        case IO_BG2Y:
        case IO_BG3X:
        case IO_BG3Y: {
            word* current = NULL;
            word* initial = NULL;
            switch (maskedaddr) {
                case IO_BG2X:
                    current = &ppu->BG2X.current.raw;
                    initial = &ppu->BG2X.initial.raw;
                    break;
                case IO_BG2Y:
                    current = &ppu->BG2Y.current.raw;
                    initial = &ppu->BG2Y.initial.raw;
                    break;
                case IO_BG3X:
                    current = &ppu->BG3X.current.raw;
                    initial = &ppu->BG3X.initial.raw;
                    break;
                case IO_BG3Y:
                    current = &ppu->BG3Y.current.raw;
                    initial = &ppu->BG3Y.initial.raw;
                    break;
                default:
                    logfatal("Fatal error- should not have ended up here.")
            }
            // Only update "current" if is VBlank
            if (!is_vblank(ppu)) {
                *current &= (~mask);
                *current |= (value & mask);
            }
            *initial &= (~mask);
            *initial |= (value & mask);
            break;
        }
        default:
            if (!is_ioreg_writable(addr)) {
                logwarn("Ignoring write to unwriteable word ioreg 0x%08X", addr)
                return;
            }
            word* ioreg = get_word_ioreg_ptr(addr);
            if (ioreg) {
                *ioreg &= (~mask);
                *ioreg |= (value & mask);
            } else {
                logwarn("Ignoring write to word ioreg 0x%08X with mask 0x%08X", addr, mask)
            }
    }
}

INLINE void write_word_ioreg(word addr, word value) {
    // 0x04XX0800 is the only address that's mirrored.
    if ((addr & 0xFF00FFFFu) == 0x04000800u) {
        addr = 0x04000800;
    }

    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable word ioreg 0x%08X", addr)
    }

    write_word_ioreg_masked(addr, value, 0xFFFFFFFF);
}

INLINE word read_word_ioreg(word addr) {
    if (!is_ioreg_readable(addr)) {
        logwarn("Returning 0 (UNREADABLE BUT VALID WORD IOREG 0x%08X)", addr)
        return 0;
    }
    word* ioreg = get_word_ioreg_ptr(addr);
    if (ioreg) {
        return *ioreg;
    } else {
        logwarn("Ignoring read from word ioreg at 0x%08X and returning 0.", addr)
        return 0;
    }
}

INLINE half read_half_ioreg(word addr) {
    if (!is_ioreg_readable(addr)) {
        logwarn("Returning 0 (UNREADABLE BUT VALID HALF IOREG 0x%08X)", addr)
        return 0;
    }
    half* ioreg = get_half_ioreg_ptr(addr, false);
    if (ioreg) {
        return *ioreg;
    } else {
        logwarn("Ignoring read from half ioreg at 0x%08X and returning 0.", addr)
        return 0;
    }
}

INLINE word open_bus(word addr) {
    word result;

    if (cpu->cpsr.thumb)
    {
        word low = cpu->pipeline[1];
        word high = cpu->pipeline[1];

        byte region = addr >> 24;

        if (region == 0 || region == 7) {
            low = cpu->pipeline[0];
        } else if (region == 3) {
            if (addr & 3) {
                low = cpu->pipeline[0];
            } else {
                high = cpu->pipeline[0];
            }
        }

        result = (high << 16) | low;
    } else {
        result = cpu->pipeline[1];
    }
    result >>= ((addr & 0b11u) << 3u);

    logwarn("RETURNING FROM OPEN BUS AT ADDRESS 0x%08X: 0x%08X", addr, result)
    return result;
}

INLINE byte inline_gba_read_byte(word addr, access_type_t access_type) {
    addr &= ~(sizeof(byte) - 1);
    half region = addr >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks += byte_half_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            if (addr < GBA_BIOS_SIZE) { // BIOS
                return gbabios_read_byte(addr);
            } else {
                return open_bus(addr);
            }
        }
        case REGION_EWRAM: {
            word index = (addr - 0x02000000) % 0x40000;
            return mem->ewram[index];
        }
        case REGION_IWRAM: {
            word index = (addr - 0x03000000) % 0x8000;
            return mem->iwram[index];
        }
        case REGION_IOREG: {
            if (addr < 0x04000400) {
                byte size = get_ioreg_size_for_addr(addr);
                if (size == 0) {
                    logwarn("Returning open bus (UNUSED BYTE IOREG 0x%08X)", addr)
                    return open_bus(addr);
                } else if (size == sizeof(half)) {
                    int ofs = addr % 2;
                    half* ioreg = get_half_ioreg_ptr(addr - ofs, false);
                    if (ioreg) {
                        return (*ioreg >> ofs) & 0xFF;
                    } else {
                        logwarn("Ignoring read from half ioreg 0x%08X", addr - ofs)
                        return 0;
                    }
                }
                else if (size > sizeof(byte)) {
                    logfatal("Reading from too-large ioreg (%d) as byte at 0x%08X", size, addr)
                }
                if (!is_ioreg_readable(addr)) {
                    logwarn("Returning 0 (UNREADABLE BUT VALID BYTE IOREG 0x%08X)", addr)
                    return 0;
                }
                return read_byte_ioreg(addr);
            } else {
                logwarn("Tried to read from 0x%08X", addr)
                return open_bus(addr);
            }
        }
        case REGION_PRAM: {
            word index = (addr - 0x5000000) % 0x400;
            return ppu->pram[index];
        }
        case REGION_VRAM: {
            word index = addr - 0x06000000;
            index %= VRAM_SIZE;
            return ppu->vram[index];
        }
        case REGION_OAM: {
            word index = addr - 0x07000000;
            index %= OAM_SIZE;
            return ppu->oam[index];
        }
        case REGION_GAMEPAK0_L:
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L: {
            word index = addr & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return mem->rom[index];
            } else {
                return open_bus(addr);
            }

        }

        case REGION_GAMEPAK2_H:
            if (bus->backup_type == EEPROM) {
                if (mem->rom_size <= 0x1000000 || addr >= 0xDFFFF00) {
                    return 1;
                }
            }
            word index = addr & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return mem->rom[index];
            } else {
                return open_bus(addr);
            }

        case REGION_SRAM:
            switch (bus->backup_type) {
                case SRAM:
                    return mem->backup[addr & 0x7FFF];
                case FLASH64K:
                case FLASH128K:
                    return read_byte_flash(mem, addr, bus->backup_type);
                default: break;
            }
            return 0;

        case REGION_SRAM_MIRR:
            if (bus->backup_type == SRAM) {
                return mem->backup[addr & 0x7FFF];
            }
            return 0;
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", addr)
    }
    return open_bus(addr);
}

byte gba_read_byte(word addr, access_type_t access_type) {
    return inline_gba_read_byte(addr, access_type);
}

INLINE half inline_gba_read_half(word address, access_type_t access_type) {
    address &= ~(sizeof(half) - 1);
    half region = address >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks += byte_half_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            if (address < GBA_BIOS_SIZE) { // BIOS
                return gbabios_read_byte(address) | (gbabios_read_byte(address + 1) << 8);
            } else {
                return open_bus(address);
            }
        }
        case REGION_EWRAM: {
            word index = (address - 0x02000000) % 0x40000;
            return half_from_byte_array(mem->ewram, index);
        }
        case REGION_IWRAM: {
            word index = (address - 0x03000000) % 0x8000;
            return half_from_byte_array(mem->iwram, index);
        }
        case REGION_IOREG: {
            if (address < 0x04000400) {
                byte size = get_ioreg_size_for_addr(address);
                switch (size) {
                    case 0:
                        logwarn("Returning open bus (UNUSED HALF IOREG 0x%08X)", address)
                        return open_bus(address);
                    case sizeof(byte):
                        return read_byte_ioreg(address) | (read_byte_ioreg(address + 1) << 8);
                    case sizeof(half):
                        return read_half_ioreg(address);
                    case sizeof(word):
                        logfatal("Reading word ioreg from gba_read_half()")
                }
            } else {
                logwarn("Tried to read from 0x%08X", address)
                return open_bus(address);
            }
        }
        case REGION_PRAM: {
            word index = (address - 0x5000000) % 0x400;
            return half_from_byte_array(ppu->pram, index);
        }
        case REGION_VRAM: {
            word index = (address - 0x06000000) % VRAM_SIZE;
            return half_from_byte_array(ppu->vram, index);
        }
        case REGION_OAM: {
            word index = (address - 0x07000000) % OAM_SIZE;
            return half_from_byte_array(ppu->oam, index);
        }
        case REGION_GAMEPAK0_L:
            if (address >= 0x080000C4 && address <= 0x080000C8 && bus->allow_gpio_read) {
                return gpio_read(address);
            }
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L: {
            word index = address & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return half_from_byte_array(mem->rom, index);
            } else {
                return open_bus(address);
            }
        }

        case REGION_GAMEPAK2_H:
            if (bus->backup_type == EEPROM) {
                if (mem->rom_size <= 0x1000000 || address >= 0xDFFFF00) {
                    return 1;
                }
            }
            word index = address & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return half_from_byte_array(mem->rom, index);
            } else {
                return open_bus(address);
            }

        case REGION_SRAM:
            switch (bus->backup_type) {
                case SRAM:
                    return half_from_byte_array(mem->backup, address & 0x7FFF);
                case FLASH64K:
                case FLASH128K:
                    logfatal("gba_read_half from FLASH!")
                default: break;
            }
            return 0;

        case REGION_SRAM_MIRR:
            if (bus->backup_type == SRAM) {
                return half_from_byte_array(mem->backup, address & 0x7FFF);
            }
            return 0;
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", address)
    }
    return open_bus(address);
}

half gba_read_half(word address, access_type_t access_type) {
    return inline_gba_read_half(address, access_type);
}

INLINE bool is_bitmap() {
    int mode = ppu->DISPCNT.mode;
    return mode >= 3 && mode < 6;
}

INLINE bool is_bg(word address) {
    if (is_bitmap()) {
        return address >= 0x6000000 && address <= 0x6013FFF;
    } else {
        return address >= 0x6000000 && address <= 0x600FFFF;
    }
}

void gba_write_byte(word addr, byte value, access_type_t access_type) {
    addr &= ~(sizeof(byte) - 1);
    half region = addr >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks = byte_half_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            logwarn("Tried to write to the BIOS!")
            break;
        }
        case REGION_EWRAM: {
            word index = (addr - 0x02000000) % 0x40000;
            mem->ewram[index] = value;
            break;
        }
        case REGION_IWRAM: {
            word index = (addr - 0x03000000) % 0x8000;
            mem->iwram[index] = value;
            break;
        }
        case REGION_IOREG: {
            write_byte_ioreg(addr, value);
            break;
        }
        case REGION_PRAM: {
            word index = (addr - 0x5000000) % 0x400;
            word lower_index = index & 0xFFFFFFFE;
            word upper_index = lower_index + 1;
            ppu->pram[lower_index] = value;
            ppu->pram[upper_index] = value;
            break;
        }
        case REGION_VRAM: {
            word index = addr - 0x06000000;
            index %= VRAM_SIZE;
            // Special case for single byte writes to VRAM, OBJ writes are ignored.
            if (is_bg(addr)) {
                word lower_index = index & 0xFFFFFFFE;
                word upper_index = lower_index + 1;
                ppu->vram[lower_index] = value;
                ppu->vram[upper_index] = value;
            }
            break;
        }
        case REGION_OAM: {
            break; // 8 bit writes to OAM are ignored
        }
        case REGION_GAMEPAK0_L:
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L:
        case REGION_GAMEPAK2_H: {
            logdebug("Ignoring write to cartridge space address 0x%08X", addr)
            break;
        }
        case REGION_SRAM:
        case REGION_SRAM_MIRR:
            // Backup space
            switch (bus->backup_type) {
                case SRAM:
                    mem->backup[addr & 0x7FFF] = value;
                    mem->backup_dirty = true;
                    break;
                case UNKNOWN:
                    logwarn("Tried to access backup when backup type unknown!")
                    break;
                case EEPROM:
                    logfatal("Backup type EEPROM unimplemented!")
                case FLASH64K:
                case FLASH128K:
                    write_byte_flash(mem, bus, addr, value);
                    break;
                default:
                    logfatal("Unknown backup type index %d!", bus->backup_type)
            }
            break;
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", addr)
    }
}

void gba_write_half(word addr, half value, access_type_t access_type) {
    addr &= ~(sizeof(half) - 1);
    half region = addr >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks = byte_half_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            logwarn("Tried to write to the BIOS!")
            break;
        }
        case REGION_EWRAM: {
            word index = (addr - 0x02000000) % 0x40000;
            half_to_byte_array(mem->ewram, index, value);
            break;
        }
        case REGION_IWRAM: {
            word index = (addr - 0x03000000) % 0x8000;
            half_to_byte_array(mem->iwram, index, value);
            break;
        }
        case REGION_IOREG: {
            byte ioreg_size = get_ioreg_size_for_addr(addr);
            if (ioreg_size == sizeof(word)) {
                word offset = (addr % sizeof(word));
                word shifted_value = value;
                shifted_value <<= (offset * 8);
                write_word_ioreg_masked(addr - offset, shifted_value, 0xFFFF << (offset * 8));
            } else if (ioreg_size == sizeof(half)) {
                write_half_ioreg(addr, value);
                return;
            } else if (ioreg_size == 0) {
                // Unused io register
                logwarn("Unused half size ioregister 0x%08X", addr)
                return;
            }
            break;
        }
        case REGION_PRAM: {
            word index = (addr - 0x5000000) % 0x400;
            half_to_byte_array(ppu->pram, index, value);
            break;
        }
        case REGION_VRAM: {
            word index = addr - 0x06000000;
            index %= VRAM_SIZE;
            half_to_byte_array(ppu->vram, index, value);
            break;
        }
        case REGION_OAM: {
            word index = addr - 0x07000000;
            index %= OAM_SIZE;
            ppu->oam[index] = value;
            half_to_byte_array(ppu->oam, index, value);
            break;
        }
        case REGION_GAMEPAK0_L:
            if (addr >= 0x080000C4 && addr <= 0x080000C8) {
                gpio_write(addr, value);
                return;
            }
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L:
        case REGION_GAMEPAK2_H: {
            logdebug("Ignoring write to cartridge space address 0x%08X", addr)
            break;
        }
        case REGION_SRAM:
        case REGION_SRAM_MIRR:
            // Backup space
            switch (bus->backup_type) {
                case SRAM:
                    half_to_byte_array(mem->backup, addr & 0x7FFF, value);
                    mem->backup_dirty = true;
                    break;
                case UNKNOWN:
                    logwarn("Tried to access backup when backup type unknown!")
                    break;
                case EEPROM:
                    logfatal("Backup type EEPROM unimplemented!")
                case FLASH64K:
                case FLASH128K:
                    logfatal("Writing half to flash unimplemented")
                    break;
                default:
                    logfatal("Unknown backup type index %d!", bus->backup_type)
            }
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", addr)
    }
}

word gba_read_word(word address, access_type_t access_type) {
    address &= ~(sizeof(word) - 1);
    half region = address >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks += word_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            if (address < GBA_BIOS_SIZE) { // BIOS
                return gbabios_read_byte(address)
                       | (gbabios_read_byte(address + 1) << 8)
                       | (gbabios_read_byte(address + 2) << 16)
                       | (gbabios_read_byte(address + 3) << 24);
            } else {
                return open_bus(address);
            }
        }

        case REGION_EWRAM: {
            word index = (address - 0x02000000) % 0x40000;
            return word_from_byte_array(mem->ewram, index);
        }
        case REGION_IWRAM: {
            word index = (address - 0x03000000) % 0x8000;
            return word_from_byte_array(mem->iwram, index);
        }
        case REGION_IOREG: {
            if (address < 0x04000400) {
                byte size = get_ioreg_size_for_addr(address);
                switch (size) {
                    case 0:
                        logwarn("Returning open bus (UNUSED WORD IOREG 0x%08X)", address)
                        return open_bus(address);
                    case sizeof(byte):
                        return read_byte_ioreg(address)
                               | (read_byte_ioreg(address + 1) << 8)
                               | (read_byte_ioreg(address + 2) << 16)
                               | (read_byte_ioreg(address + 3) << 24);
                    case sizeof(half):
                        return read_half_ioreg(address) | (read_half_ioreg(address + 2) << 16);
                    case sizeof(word):
                        return read_word_ioreg(address);
                }
            } else {
                logwarn("Tried to read from 0x%08X", address)
                return open_bus(address);
            }
        }
        case REGION_PRAM: {
            word index = (address - 0x5000000) % 0x400;
            return word_from_byte_array(ppu->pram, index);
        }
        case REGION_VRAM: {
            word index = (address - 0x06000000) % VRAM_SIZE;
            return word_from_byte_array(ppu->vram, index);
        }
        case REGION_OAM: {
            word index = (address - 0x07000000) % OAM_SIZE;
            return word_from_byte_array(ppu->oam, index);
        }
        case REGION_GAMEPAK0_L:
            if (bus->allow_gpio_read) {
                switch (address) {
                    case 0x080000C4:
                        return gpio_read(0x080000C4) | (gpio_read(0x080000C6) << 16);
                    case 0x080000C8:
                        return gpio_read(0x080000C8);
                    default:
                        break;
                }
            }
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L: {
            word index = address & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return word_from_byte_array(mem->rom, index);
            } else {
                return open_bus(address);
            }
        }

        case REGION_GAMEPAK2_H:
            if (bus->backup_type == EEPROM) {
                if (mem->rom_size <= 0x1000000 || address >= 0xDFFFF00) {
                    return 1;
                }
            }
            word index = address & 0x1FFFFFF;
            if (index < mem->rom_size) {
                return word_from_byte_array(mem->rom, index);
            } else {
                return open_bus(address);
            }

        case REGION_SRAM:
            switch (bus->backup_type) {
                case SRAM:
                    return word_from_byte_array(mem->backup, address & 0x7FFF);
                case FLASH64K:
                case FLASH128K:
                    logfatal("gba_read_word from FLASH!")
                default: break;
            }
            return 0;

        case REGION_SRAM_MIRR:
            if (bus->backup_type == SRAM) {
                return word_from_byte_array(mem->backup, address & 0x7FFF);
            }
            return 0;
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", address)
    }
    return open_bus(address);
}

void gba_write_word(word addr, word value, access_type_t access_type) {
    addr &= ~(sizeof(word) - 1);
    half region = addr >> 24;
    if (access_type != ACCESS_UNKNOWN) cpu->this_step_ticks += word_cycles[region];
    switch (region) {
        case REGION_BIOS: {
            logwarn("Tried to write to the BIOS!")
            break;
        }
        case REGION_EWRAM: {
            word index = (addr - 0x02000000) % 0x40000;
            word_to_byte_array(mem->ewram, index, value);
            break;
        }
        case REGION_IWRAM: {
            word index = (addr - 0x03000000) % 0x8000;
            word_to_byte_array(mem->iwram, index, value);
            break;
        }
        case REGION_IOREG: {
            if (addr < 0x04000400) {
                byte size = get_ioreg_size_for_addr(addr);
                switch (size) {
                    case 0:
                        return;
                    case sizeof(byte):
                        write_byte_ioreg(addr, value & 0xFF);
                        write_byte_ioreg(addr + 1, (value >> 8) & 0xFF);
                        write_byte_ioreg(addr + 2, (value >> 16) & 0xFF);
                        write_byte_ioreg(addr + 3, (value >> 24) & 0xFF);
                        break;
                    case sizeof(half):
                        write_half_ioreg(addr, value & 0xFFFF);
                        write_half_ioreg(addr + 2, (value >> 16) & 0xFFFF);
                        break;
                    case sizeof(word):
                        write_word_ioreg(addr, value);
                }
            }
            break;
        }
        case REGION_PRAM: {
            word index = (addr - 0x5000000) % 0x400;
            word_to_byte_array(ppu->pram, index, value);
            break;
        }
        case REGION_VRAM: {
            word index = addr - 0x06000000;
            index %= VRAM_SIZE;
            word_to_byte_array(ppu->vram, index, value);
            break;
        }
        case REGION_OAM: {
            word index = addr - 0x07000000;
            index %= OAM_SIZE;
            ppu->oam[index] = value;
            word_to_byte_array(ppu->oam, index, value);
            break;
        }
        case REGION_GAMEPAK0_L:
            switch (addr) {
                case 0x080000C4:
                    gpio_write(0x080000C4, value & 0xFFFF);
                    gpio_write(0x080000C6, (value >> 16) & 0xFFFF);
                    return;
                case 0x080000C8:
                    gpio_write(0x080000C8, value & 0xFFFF);
                    return;
                default:
                    break;
            }
        case REGION_GAMEPAK0_H:
        case REGION_GAMEPAK1_L:
        case REGION_GAMEPAK1_H:
        case REGION_GAMEPAK2_L:
        case REGION_GAMEPAK2_H: {
            logdebug("Ignoring write to cartridge space address 0x%08X", addr)
            break;
        }
        case REGION_SRAM:
        case REGION_SRAM_MIRR:
            // Backup space
            switch (bus->backup_type) {
                case SRAM:
                    word_to_byte_array(mem->backup, addr & 0x7FFF, value);
                    mem->backup_dirty = true;
                    break;
                case UNKNOWN:
                    logwarn("Tried to access backup when backup type unknown!")
                    break;
                case EEPROM:
                    logfatal("Backup type EEPROM unimplemented!")
                case FLASH64K:
                case FLASH128K:
                    logfatal("Writing word to flash unimplemented")
                    break;
                default:
                    logfatal("Unknown backup type index %d!", bus->backup_type)
            }
        default:
            logfatal("Access to unknown memory region for address: 0x%08X", addr)
    }
}
