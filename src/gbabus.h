#ifndef __GBABUS_H__
#define __GBABUS_H__

#include "common/util.h"

#include "gbamem.h"
#include "arm7tdmi/arm7tdmi.h"
#include "ppu.h"

typedef union interrupt_enable {
    struct {
        bool lcd_vblank:1;
        bool lcd_hblank:1;
        bool lcd_vcounter_match:1;
        bool timer0_overflow:1;
        bool timer1_overflow:1;
        bool timer2_overflow:1;
        bool timer3_overflow:1;
        bool serial_communication:1;
        bool dma_0:1;
        bool dma_1:1;
        bool dma_2:1;
        bool dma_3:1;
        bool keypad:1;
        bool gamepak:1;
        unsigned:2; // Unused
    };
    half raw;
} interrupt_enable_t;

typedef struct gbabus {
    bool interrupt_master_enable;
    interrupt_enable_t interrupt_enable;
} gbabus_t;

void init_gbabus(gbamem_t* new_mem, arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu);
byte gba_read_byte(word addr);
half gba_read_half(word address);
void gba_write_byte(word addr, byte value);
void gba_write_half(word address, half value);
word gba_read_word(word address);
void gba_write_word(word address, word value);
#endif
