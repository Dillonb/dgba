#include "gbabus.h"
#include "ioreg_util.h"
#include "gbamem.h"
#include "common/log.h"
#include "gbabios.h"

gbamem_t* mem;
arm7tdmi_t* cpu;
gba_ppu_t* ppu;

gbabus_t state;

word open_bus(word addr);

void init_gbabus(gbamem_t* new_mem, arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu) {
    mem = new_mem;
    cpu = new_cpu;
    ppu = new_ppu;

    state.interrupt_master_enable.raw = 0;
    state.interrupt_enable.raw = 0;
    state.KEYINPUT.raw = 0xFFFF;
    state.SOUNDBIAS.raw = 0x0200;
}

KEYINPUT_t* get_keyinput() {
    return &state.KEYINPUT;
}

void request_interrupt(gba_interrupt_t interrupt) {
    if (state.interrupt_master_enable.enable) {
        switch (interrupt) {
            case IRQ_VBLANK:
                if (state.interrupt_enable.lcd_vblank) {
                    logfatal("VBlank interrupt unhandled!")
                } else {
                    logwarn("VBlank interrupt blocked by IE")
                }
                break;
            case IRQ_HBLANK:
                if (state.interrupt_enable.lcd_hblank) {
                    logfatal("HBlank interrupt unhandled!")
                } else {
                    logwarn("HBlank interrupt blocked by IE")
                }
                break;
            default:
                logfatal("Unknown interrupt index %d requested!", interrupt)
        }
    } else {
        logwarn("Interrupt blocked by IME")
    }
}

void write_half_ioreg_masked(word addr, half value, half mask);
void write_word_ioreg_masked(word addr, word value, word mask);

void write_byte_ioreg(word addr, byte value) {
    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable byte ioreg 0x%08X", addr)
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
            default:
                logfatal("Write to unknown (but valid) byte ioreg addr 0x%08X == 0x%02X", addr, value)
        }
    }
}

byte read_byte_ioreg(word addr) {
    logfatal("Reading byte ioreg at 0x%08X", addr)
}

half* get_half_ioreg_ptr(word addr) {
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
        case IO_IE: return &state.interrupt_enable.raw;
        case IO_DMA0CNT_L: return &state.DMA0CNT_L.raw;
        case IO_DMA0CNT_H: return &state.DMA0CNT_H.raw;
        case IO_DMA1CNT_L: return &state.DMA1CNT_L.raw;
        case IO_DMA1CNT_H: return &state.DMA1CNT_H.raw;
        case IO_DMA2CNT_L: return &state.DMA2CNT_L.raw;
        case IO_DMA2CNT_H: return &state.DMA2CNT_H.raw;
        case IO_DMA3CNT_L: return &state.DMA3CNT_L.raw;
        case IO_DMA3CNT_H: return &state.DMA3CNT_H.raw;
        case IO_KEYINPUT: return &state.KEYINPUT.raw;
        case IO_RCNT: return &state.RCNT.raw;
        case IO_JOYCNT: return &state.JOYCNT.raw;
        case IO_IME: return &state.interrupt_master_enable.raw;
        case IO_SOUNDBIAS: return &state.SOUNDBIAS.raw;

        case IO_IF:
            logwarn("Ignoring access to IF register")
            return NULL;
        case IO_WAITCNT:
            logwarn("Ignoring access to WAITCNT register")
            return NULL;
        case IO_UNDOCUMENTED_GREEN_SWAP:
            logwarn("Ignoring access to Green Swap register")
            return NULL;
        case IO_VCOUNT:
            logwarn("Access to read-only VCOUNT register, ignoring!")
            return NULL;
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

        default:
            logfatal("Access to unknown (but valid) half ioreg addr 0x%08X", addr)
    }
}

void write_half_ioreg_masked(word addr, half value, half mask) {
    half* ioreg = get_half_ioreg_ptr(addr);
    if (ioreg) {
        switch (addr & 0xFFF) {
            case IO_DISPSTAT:
                mask &= 0b1111111111111000; // Last 3 bits are read-only
                break;
            case IO_IME:
                mask = 0b1;
                break;
            default:
                break; // No special case
        }
        *ioreg &= (~mask);
        *ioreg |= (value & mask);
    } else {
        logwarn("Ignoring write to word ioreg")
    }
}

void write_half_ioreg(word addr, half value) {
    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable byte ioreg 0x%08X", addr)
    }
    // Write to the whole thing
    write_half_ioreg_masked(addr, value, 0xFFFF);
}

word* get_word_ioreg_ptr(word addr) {
    unimplemented(get_ioreg_size_for_addr(addr) != sizeof(word), "Trying to get the address of a wrong-sized word ioreg")
    switch (addr & 0xFFF) {
        case IO_BG2X:     return &ppu->BG2X.raw;
        case IO_BG2Y:     return &ppu->BG2Y.raw;
        case IO_BG3X:     return &ppu->BG3X.raw;
        case IO_BG3Y:     return &ppu->BG3Y.raw;
        case IO_DMA0SAD:  return &state.DMA0SAD.raw;
        case IO_DMA0DAD:  return &state.DMA0DAD.raw;
        case IO_DMA1SAD:  return &state.DMA1SAD.raw;
        case IO_DMA1DAD:  return &state.DMA1DAD.raw;
        case IO_DMA2SAD:  return &state.DMA2SAD.raw;
        case IO_DMA2DAD:  return &state.DMA2DAD.raw;
        case IO_DMA3SAD:  return &state.DMA3SAD.raw;
        case IO_DMA3DAD:  return &state.DMA3DAD.raw;
        default: logfatal("Tried to get the address of an unknown (but valid) word ioreg addr: 0x%08X", addr)
    }
}

void write_word_ioreg_masked(word addr, word value, word mask) {
    word* ioreg = get_word_ioreg_ptr(addr);
    if (ioreg) {
        *ioreg &= (~mask);
        *ioreg |= (value & mask);
    } else {
        logwarn("Ignoring write to word ioreg")
    }
}

void write_word_ioreg(word addr, word value) {
    // 0x04XX0800 is the only address that's mirrored.
    if ((addr & 0xFF00FFFFu) == 0x04000800u) {
        addr = 0xFF00FFFFu;
    }

    if (!is_ioreg_writable(addr)) {
        logwarn("Ignoring write to unwriteable word ioreg 0x%08X", addr)
    }

    write_word_ioreg_masked(addr, value, 0xFFFFFFFF);
}

word read_word_ioreg(word addr) {
    if (!is_ioreg_readable(addr)) {
        logwarn("Returning from open bus (UNREADABLE BUT VALID WORD IOREG 0x%08X)", addr)
        return open_bus(addr);
    }
    logfatal("read from unknown (but valid) word ioreg addr 0x%08x", addr)
}

half read_half_ioreg(word addr) {
    if (!is_ioreg_readable(addr)) {
        logwarn("Returning from open bus (UNREADABLE BUT VALID HALF IOREG 0x%08X)", addr)
        return open_bus(addr);
    }
    half* ioreg = get_half_ioreg_ptr(addr);
    if (ioreg) {
        return *ioreg;
    } else {
        logwarn("Ignoring read from half ioreg at 0x%08X and returning 0.", addr)
        return 0;
    }
}

bool is_open_bus(word address) {
    if (address < GBA_BIOS_SIZE) {
        return false;
    } else if (address < 0x01FFFFFF) {
        return true;
    } else if (address < 0x08000000) {
        return false;
    } else if (address < 0x08000000 + mem->rom_size) {
        return false;
    } else {
        return true;
    }
}

word open_bus(word addr) {
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

byte gba_read_byte(word addr) {
    addr &= ~(sizeof(byte) - 1);
    if (addr < GBA_BIOS_SIZE) { // BIOS
        return gbabios_read_byte(addr);
    } else if (addr < 0x01FFFFFF) {
        return open_bus(addr);
    } else if (addr < 0x03000000) { // EWRAM
        word index = (addr - 0x02000000) % 0x40000;
        return mem->ewram[index];
    } else if (addr < 0x04000000) { // IWRAM
        word index = (addr - 0x03000000) % 0x8000;
        return mem->iwram[index];
    } else if (addr < 0x04000400) {
        byte size = get_ioreg_size_for_addr(addr);
        if (size == 0) {
            logwarn("Returning open bus (UNUSED BYTE IOREG 0x%08X)", addr)
            return open_bus(addr);
        } else if (size == sizeof(half)) {
            half* ioreg = get_half_ioreg_ptr(addr);
            int ofs = addr % 2;
            return (*ioreg >> ofs) & 0xFF;
        }
        else if (size > sizeof(byte)) {
            logfatal("Reading from too-large ioreg (%d) as byte at 0x%08X", size, addr)
        }
        if (!is_ioreg_readable(addr)) {
            logwarn("Returning 0 (UNREADABLE BUT VALID BYTE IOREG 0x%08X)", addr)
            return 0;
        }
        return read_byte_ioreg(addr);
    } else if (addr < 0x05000000) {
        logwarn("Tried to read from 0x%08X", addr)
        unimplemented(1, "Tried to read from unused portion of general internal memory")
    } else if (addr < 0x06000000) { // Palette RAM
        word index = (addr - 0x5000000) % 0x400;
        return ppu->pram[index];
    } else if (addr < 0x07000000) {
        word index = addr - 0x06000000;
        unimplemented(index > 0x17FFF, "VRAM mirroring")
        return ppu->vram[index];
    } else if (addr < 0x08000000) {
        word index = addr - 0x08000000;
        index %= OAM_SIZE;
        return ppu->oam[index];
    } else if (addr < 0x0E00FFFF) {
        // Cartridge
        word adjusted = addr - 0x08000000;
        if (adjusted > mem->rom_size) {
            return open_bus(addr);
        } else {
            return mem->rom[adjusted];
        }
    }

    return open_bus(addr);
}

half gba_read_half(word address) {
    address &= ~(sizeof(half) - 1);
    if (is_ioreg(address)) {
        byte ioreg_size = get_ioreg_size_for_addr(address);
        unimplemented(ioreg_size > sizeof(half), "Reading from a too-large ioreg from gba_read_half")
        if (ioreg_size == sizeof(half)) {
            return read_half_ioreg(address);
        } else if (ioreg_size == 0) {
            // Unused io register
            logfatal("Read from unused half size ioregister!")
        }
    }

    if (is_open_bus(address)) {
        return open_bus(address);
    }
    byte lower = gba_read_byte(address);
    byte upper = gba_read_byte(address + 1);

    return (upper << 8u) | lower;
}

void gba_write_byte(word addr, byte value) {
    addr &= ~(sizeof(byte) - 1);
    if (addr < GBA_BIOS_SIZE) {
        logwarn("Tried to write to the BIOS!")
    } else if (addr < 0x01FFFFFF) {
        logwarn("Tried to write to unused section of RAM in between bios and WRAM")
    } else if (addr < 0x03000000) { // EWRAM
        word index = (addr - 0x02000000) % 0x40000;
        mem->ewram[index] = value;
    } else if (addr < 0x04000000) { // IWRAM
        word index = (addr - 0x03000000) % 0x8000;
        mem->iwram[index] = value;
    } else if (addr < 0x04000400) {
        write_byte_ioreg(addr, value);
    } else if (addr < 0x05000000) {
        logwarn("Tried to write to 0x%08X", addr)
        unimplemented(1, "Tried to write to unused portion of general internal memory")
    } else if (addr < 0x06000000) { // Palette RAM
        word index = (addr - 0x5000000) % 0x400;
        ppu->pram[index] = value;
    } else if (addr < 0x07000000) {
        word index = addr - 0x06000000;
        unimplemented(index > 0x17FFF, "VRAM mirroring")
        ppu->vram[index] = value;
    } else if (addr < 0x08000000) {
        word index = addr - 0x08000000;
        index %= OAM_SIZE;
        ppu->oam[index] = value;
    } else if (addr < 0x0E00FFFF) {
        logwarn("Tried to write to 0x%08X", addr)
    } else {
        logwarn("Ignoring write to high address! addr: 0x%08X", addr)
    }
}

void gba_write_half(word address, half value) {
    address &= ~(sizeof(half) - 1);
    if (is_ioreg(address)) {
        byte ioreg_size = get_ioreg_size_for_addr(address);
        if (ioreg_size == sizeof(word)) {
            word offset = (address % sizeof(word));
            word shifted_value = value;
            shifted_value <<= (offset * 8);
            write_word_ioreg_masked(address - offset, shifted_value, 0xFFFF << (offset * 8));
        } else if (ioreg_size == sizeof(half)) {
            write_half_ioreg(address, value);
            return;
        } else if (ioreg_size == 0) {
            // Unused io register
            logwarn("Unused half size ioregister 0x%08X", address)
            return;
        }
    }

    byte lower = value & 0xFFu;
    byte upper = (value & 0xFF00u) >> 8u;
    gba_write_byte(address, lower);
    gba_write_byte(address + 1, upper);
}

word gba_read_word(word address) {
    address &= ~(sizeof(word) - 1);

    if (is_ioreg(address)) {
        byte ioreg_size = get_ioreg_size_for_addr(address);
        if(ioreg_size == sizeof(word)) {
            return read_word_ioreg(address);
        } else if (ioreg_size == 0) {
            logfatal("Read from unused word size ioregister!")
        }
        // Otherwise, it'll be smaller than a word, and we'll read each part from the respective registers.
    }
    if (is_open_bus(address)) {
        return open_bus(address);
    }
    word lower = gba_read_half(address);
    word upper = gba_read_half(address + 2);

    return (upper << 16u) | lower;
}

void gba_write_word(word address, word value) {
    address &= ~(sizeof(word) - 1);
    if (is_ioreg(address)) {
        byte ioreg_size = get_ioreg_size_for_addr(address);
        if(ioreg_size == sizeof(word)) {
            write_word_ioreg(address, value);
        } else if (ioreg_size == 0) {
            logwarn("Unused word size ioregister!")
            // Unused io register
            return;
        }
        // Otherwise, it'll be smaller than a word, and we'll write each part to the respective registers.
    }

    half lower = (value & 0xFFFFu);
    half upper = (value & 0xFFFF0000u) >> 16u;

    gba_write_half(address, lower);
    gba_write_half(address + 2, upper);
}

