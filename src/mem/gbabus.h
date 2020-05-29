#ifndef __GBABUS_H__
#define __GBABUS_H__

#include "../common/util.h"

#include "gbamem.h"
#include "../arm7tdmi/arm7tdmi.h"
#include "../graphics/ppu.h"
#include "../audio/audio.h"

INLINE word word_from_byte_array(byte* arr, int index) {
    word* warr = (word*)arr;
    return warr[index / sizeof(word)];
}

INLINE half half_from_byte_array(byte* arr, int index) {
    half* warr = (half*)arr;
    return warr[index / sizeof(half)];
}

INLINE void word_to_byte_array(byte* arr, int index, word value) {
    word* warr = (word*)arr;
    warr[index / sizeof(word)] = value;
}

INLINE void half_to_byte_array(byte* arr, int index, half value) {
    half* warr = (half*)arr;
    warr[index / sizeof(half)] = value;
}

typedef enum gba_interrupt {
    IRQ_VBLANK,
    IRQ_HBLANK,
    IRQ_VCOUNT,
    IRQ_TIMER0,
    IRQ_TIMER1,
    IRQ_TIMER2,
    IRQ_TIMER3,
    IRQ_DMA0,
    IRQ_DMA1,
    IRQ_DMA2,
    IRQ_DMA3

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

typedef union KEYCNT {
    half raw;
} KEYCNT_t;

// MOVE THIS TO AN APPROPRIATE PLACE
typedef union SOUNDBIAS {
    half raw;
} SOUNDBIAS_t;

typedef union TMCNT_L {
    half raw;
    half timer_reload;
}TMCNT_L_t;

typedef struct TMINT {
    half value;
    half ticks;
} TMINT_t;

typedef union TMCNT_H {
    half raw;
    struct {
        unsigned frequency:2;
        bool cascade:1;
        unsigned:3;
        bool timer_irq_enable:1;
        bool start:1;
        unsigned:8;
    };
} TMCNT_H_t;

typedef union WAITCNT {
    struct {
        byte sram_wait:2;
        byte wait_state_0_nonsequential:2;
        byte wait_state_0_sequential:1;
        byte wait_state_1_nonsequential:2;
        byte wait_state_1_sequential:1;
        byte wait_state_2_nonsequential:2;
        byte wait_state_2_sequential:1;
        byte phi_terminal:2;
        byte:1;
        bool prefetch_buffer_enable:1;
        bool is_cgb_gamepak:1;
    };
    half raw;
} WAITCNT_t;

typedef union IF {
    struct {
        bool vblank:1;
        bool hblank:1;
        bool vcount:1;
        bool timer0:1;
        bool timer1:1;
        bool timer2:1;
        bool timer3:1;
        bool sio:1;
        bool dma0:1;
        bool dma1:1;
        bool dma2:1;
        bool dma3:1;
        bool keypad:1;
        bool gamepak:1;
        unsigned:2;
    };
    half raw;
} IF_t;

typedef enum backup_type {
    UNKNOWN,
    SRAM,
    EEPROM,
    FLASH64K,
    FLASH128K
} backup_type_t;

typedef enum gpio_device {
    RTC
} gpio_device_t;

typedef union gpio_port {
    byte raw;
    struct {
        bool bit_0:1;
        bool bit_1:1;
        bool bit_2:1;
        bool bit_3:1;
        byte:4;
    };
    struct {
        bool rtc_sck:1;
        bool rtc_sio:1;
        bool rtc_cs:1;
        byte:5;
    };
} gpio_port_t;

typedef enum rtc_state {
    RTC_READY,
    RTC_COMMAND_MODE_1,
    RTC_COMMAND_MODE_2,
    RTC_DATA_READ,
    RTC_DATA_WRITE
} rtc_state_t;

typedef union rtc_control {
    byte raw;
    struct {
        unsigned:1; // Unused
        unsigned:1; // IRQ duty/hold related? (GBATek doesn't know)
        unsigned:1; // Unused
        bool per_minute_irq:1;
        unsigned:1; // Unused
        bool:1; // Unknown
        bool mode_24h:1;
        bool poweroff:1;
    };
} rtc_control_t;

typedef struct rtc {
    rtc_state_t state;
    byte command_buffer;
    int current_command_bit;
    uint64_t buffer;
    uint64_t write_mask;
    byte reg;
    rtc_control_t control_reg;
} rtc_t;

typedef struct gbabus {
    interrupt_master_enable_t interrupt_master_enable;
    interrupt_master_enable_t ime_temp;
    interrupt_enable_t interrupt_enable;
    IF_t IF;
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
    KEYCNT_t KEYCNT;

    SOUNDBIAS_t SOUNDBIAS;

    bool TMSTART[4];
    byte TMACTIVE[4];
    byte num_active_timers;
    TMCNT_L_t TMCNT_L[4];
    TMCNT_H_t TMCNT_H[4];
    TMINT_t TMINT[4];

    WAITCNT_t WAITCNT;

    backup_type_t backup_type;
    byte gpio_read_mask;
    byte gpio_write_mask;
    bool allow_gpio_read;
    gpio_device_t gpio_device;
    gpio_port_t port;
    rtc_t rtc;
} gbabus_t;

KEYINPUT_t* get_keyinput();

gbabus_t* init_gbabus();
byte gba_read_byte(word addr, access_type_t access_type);
half gba_read_half(word address, access_type_t access_type);
void gba_write_byte(word addr, byte value, access_type_t access_type);
void gba_write_half(word address, half value, access_type_t access_type);
word gba_read_word(word address, access_type_t access_type);
void gba_write_word(word address, word value, access_type_t access_type);
int gba_dma();

void request_interrupt(gba_interrupt_t interrupt);
#endif
