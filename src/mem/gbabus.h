#ifndef __GBABUS_H__
#define __GBABUS_H__

#include "../common/util.h"

#include "gbamem.h"
#include "../arm7tdmi/arm7tdmi.h"
#include "../graphics/ppu.h"

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

typedef union wc_14b {
    half raw;
    struct {
        unsigned wc:14;
        unsigned:2;
    };
} wc_14b_t;

typedef union wc_16b {
    half raw;
    half wc;
} wc_16b_t;

typedef enum dma_start_time {
    Immediately,
    VBlank,
    HBlank,
    Special
} dma_start_time_t;

typedef union DMACNTH {
    struct {
        unsigned:5;
        unsigned dest_addr_control:2;
        unsigned source_addr_control:2;
        bool dma_repeat:1;
        unsigned dma_transfer_type:1;
        unsigned game_pak_drq_dma3_only:1;
        dma_start_time_t dma_start_time:2;
        bool irq_on_end_of_wc:1;
        bool dma_enable:1;
    };
    half raw;
} DMACNTH_t;

typedef struct DMAINT {
    bool previously_enabled;
    word current_source_address;
    word current_dest_address;
    word remaining;
} DMAINT_t;

typedef union RCNT {
    half raw;
} RCNT_t;

typedef union JOYCNT {
    half raw;
} JOYCNT_t;

// MOVE THIS TO AN APPROPRIATE PLACE
typedef union SOUNDBIAS {
    half raw;
} SOUNDBIAS_t;

typedef union TMCNT_L {
    half raw;
    half timer_reload;
}TMCNT_L_t;

typedef union TMCNT_H {
    half raw;
    struct {
        unsigned prescaler_selection:2;
        bool countup_timing:1;
        unsigned:3;
        bool timer_irq_enable:1;
        bool start:1;
        unsigned:8;
    };
}TMCNT_H_t;

typedef struct gbabus {
    interrupt_master_enable_t interrupt_master_enable;
    interrupt_enable_t interrupt_enable;
    KEYINPUT_t KEYINPUT;

    // DMA
    addr_27b_t DMA0SAD;
    addr_27b_t DMA0DAD;
    wc_14b_t   DMA0CNT_L;
    DMACNTH_t  DMA0CNT_H;
    DMAINT_t   DMA0INT;

    addr_28b_t DMA1SAD;
    addr_27b_t DMA1DAD;
    wc_14b_t   DMA1CNT_L;
    DMACNTH_t  DMA1CNT_H;
    DMAINT_t   DMA1INT;

    addr_28b_t DMA2SAD;
    addr_27b_t DMA2DAD;
    wc_14b_t   DMA2CNT_L;
    DMACNTH_t  DMA2CNT_H;
    DMAINT_t   DMA2INT;

    addr_28b_t DMA3SAD;
    addr_28b_t DMA3DAD;
    wc_16b_t   DMA3CNT_L;
    DMACNTH_t  DMA3CNT_H;
    DMAINT_t   DMA3INT;

    RCNT_t RCNT;
    JOYCNT_t JOYCNT;

    SOUNDBIAS_t SOUNDBIAS;

    TMCNT_L_t TM0CNT_L;
    TMCNT_H_t TM0CNT_H;
    TMCNT_L_t TM1CNT_L;
    TMCNT_H_t TM1CNT_H;
    TMCNT_L_t TM2CNT_L;
    TMCNT_H_t TM2CNT_H;
    TMCNT_L_t TM3CNT_L;
    TMCNT_H_t TM3CNT_H;

} gbabus_t;

KEYINPUT_t* get_keyinput();

void init_gbabus(gbamem_t* new_mem, arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu);
byte gba_read_byte(word addr);
half gba_read_half(word address);
void gba_write_byte(word addr, byte value);
void gba_write_half(word address, half value);
word gba_read_word(word address);
void gba_write_word(word address, word value);
int gba_dma();

void request_interrupt(gba_interrupt_t interrupt);
#endif
