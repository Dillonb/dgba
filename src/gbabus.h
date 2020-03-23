#ifndef __GBABUS_H__
#define __GBABUS_H__

#include "common/util.h"

#include "gbamem.h"
#include "arm7tdmi/arm7tdmi.h"
#include "ppu.h"

typedef enum gba_interrupt {
    IRQ_VBLANK,
    IRQ_HBLANK,
    IRQ_VCOUNT
} gba_interrupt_t;

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

typedef union interrupt_master_enable {
    struct {
        bool enable:1;
        unsigned:15;
    };
    half raw;
} interrupt_master_enable_t;

typedef union KEYINPUT {
    struct {
        bool a:1;
        bool b:1;
        bool select:1;
        bool start:1;
        bool right:1;
        bool left:1;
        bool up:1;
        bool down:1;
        bool r:1;
        bool l:1;
        unsigned:6;
    };
    uint16_t raw;
} KEYINPUT_t;

typedef struct gbabus {
    interrupt_master_enable_t interrupt_master_enable;
    interrupt_enable_t interrupt_enable;
    KEYINPUT_t KEYINPUT;
} gbabus_t;

KEYINPUT_t* get_keyinput();

void init_gbabus(gbamem_t* new_mem, arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu);
byte gba_read_byte(word addr);
half gba_read_half(word address);
void gba_write_byte(word addr, byte value);
void gba_write_half(word address, half value);
word gba_read_word(word address);
void gba_write_word(word address, word value);

void request_interrupt(gba_interrupt_t interrupt);
#endif
