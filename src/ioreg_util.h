#ifndef GBA_IOREG_UTIL_H
#define GBA_IOREG_UTIL_H

#include "common/util.h"
#include "common/log.h"

#define is_ioreg(addr) ((addr >> 24u) == 0x4u)
#define mirror_ioreg(addr) ( ((addr & 0xFF00FFFFu) == 0x04000800u) ? 0x04000800u : addr )
#define ioreg_index(addr) (mirror_ioreg(addr) - 0x04000000u)

#define IO_DISPCNT      0x000
#define IO_DISPSTAT     0x004
#define IO_VCOUNT       0x006
#define IO_BG0CNT       0x008
#define IO_BG1CNT       0x00A
#define IO_BG2CNT       0x00C
#define IO_BG3CNT       0x00E
#define IO_BG0HOFS      0x010
#define IO_BG0VOFS      0x012
#define IO_BG1HOFS      0x014
#define IO_BG1VOFS      0x016
#define IO_BG2HOFS      0x018
#define IO_BG2VOFS      0x01A
#define IO_BG3HOFS      0x01C
#define IO_BG3VOFS      0x01E
#define IO_BG2PA        0x020
#define IO_BG2PB        0x022
#define IO_BG2PC        0x024
#define IO_BG2PD        0x026
#define IO_BG2X         0x028
#define IO_BG2Y         0x02C
#define IO_BG3PA        0x030
#define IO_BG3PB        0x032
#define IO_BG3PC        0x034
#define IO_BG3PD        0x036
#define IO_BG3X         0x038
#define IO_BG3Y         0x03C
#define IO_WIN0H        0x040
#define IO_WIN1H        0x042
#define IO_WIN0V        0x044
#define IO_WIN1V        0x046
#define IO_WININ        0x048
#define IO_WINOUT       0x04A
#define IO_MOSAIC       0x04C
#define IO_BLDCNT       0x050
#define IO_BLDALPHA     0x052
#define IO_BLDY         0x054
#define IO_SOUND1CNT_L  0x060
#define IO_SOUND1CNT_H  0x062
#define IO_SOUND1CNT_X  0x064
#define IO_SOUND2CNT_L  0x068
#define IO_SOUND2CNT_H  0x06C
#define IO_SOUND3CNT_L  0x070
#define IO_SOUND3CNT_H  0x072
#define IO_SOUND3CNT_X  0x074
#define IO_SOUND4CNT_L  0x078
#define IO_SOUND4CNT_H  0x07C
#define IO_SOUNDCNT_L   0x080
#define IO_SOUNDCNT_H   0x082
#define IO_SOUNDCNT_X   0x084
#define IO_SOUNDBIAS    0x088
#define WAVE_RAM0_L     0x090
#define WAVE_RAM0_H     0x092
#define WAVE_RAM1_L     0x094
#define WAVE_RAM1_H     0x096
#define WAVE_RAM2_L     0x098
#define WAVE_RAM2_H     0x09A
#define WAVE_RAM3_L     0x09C
#define WAVE_RAM3_H     0x09E
#define IO_FIFO_A       0x0A0
#define IO_FIFO_B       0x0A4
#define IO_DMA0SAD      0x0B0
#define IO_DMA0DAD      0x0B4
#define IO_DMA0CNT_L    0x0B8
#define IO_DMA0CNT_H    0x0BA
#define IO_DMA1SAD      0x0BC
#define IO_DMA1DAD      0x0C0
#define IO_DMA1CNT_L    0x0C4
#define IO_DMA1CNT_H    0x0C6
#define IO_DMA2SAD      0x0C8
#define IO_DMA2DAD      0x0CC
#define IO_DMA2CNT_L    0x0D0
#define IO_DMA2CNT_H    0x0D2
#define IO_DMA3SAD      0x0D4
#define IO_DMA3DAD      0x0D8
#define IO_DMA3CNT_L    0x0DC
#define IO_DMA3CNT_H    0x0DE
#define IO_TM0CNT_L     0X100
#define IO_TM0CNT_H     0X102
#define IO_TM1CNT_L     0X104
#define IO_TM1CNT_H     0X106
#define IO_TM2CNT_L     0X108
#define IO_TM2CNT_H     0X10A
#define IO_TM3CNT_L     0X10C
#define IO_TM3CNT_H     0X10E
#define IO_KEYINPUT     0x130
#define IO_RCNT         0x134
#define IO_JOYCNT       0x140
#define IO_IE           0x200
#define IO_IF           0x202
#define IO_WAITCNT      0x204
#define IO_IME          0x208

#define IO_UNDOCUMENTED_GREEN_SWAP 0x002

// Offset by 0x4000000 - subtract that to get the index to this array
// In the case of >1 sizes, % by the value returned to get the base.
// 0 means the address is not used.
// All indices must be less than 0x4000804 - anything above this is either unused or a mirror of 0x4000800.
byte io_register_sizes[] = {
        // LCD I/O registers
        2,2,     // 0x4000000     R/W  DISPCNT   LCD Control
        2,2,     // 0x4000002     R/W  -         Undocumented - Green Swap
        2,2,     // 0x4000004     R/W  DISPSTAT  General LCD Status (STAT,LYC)
        2,2,     // 0x4000006     R    VCOUNT    Vertical Counter (LY)
        2,2,     // 0x4000008     R/W  BG0CNT    BG0 Control
        2,2,     // 0x400000A     R/W  BG1CNT    BG1 Control
        2,2,     // 0x400000C     R/W  BG2CNT    BG2 Control
        2,2,     // 0x400000E     R/W  BG3CNT    BG3 Control
        2,2,     // 0x4000010     W    BG0HOFS   BG0 X-Offset
        2,2,     // 0x4000012     W    BG0VOFS   BG0 Y-Offset
        2,2,     // 0x4000014     W    BG1HOFS   BG1 X-Offset
        2,2,     // 0x4000016     W    BG1VOFS   BG1 Y-Offset
        2,2,     // 0x4000018     W    BG2HOFS   BG2 X-Offset
        2,2,     // 0x400001A     W    BG2VOFS   BG2 Y-Offset
        2,2,     // 0x400001C     W    BG3HOFS   BG3 X-Offset
        2,2,     // 0x400001E     W    BG3VOFS   BG3 Y-Offset
        2,2,     // 0x4000020     W    BG2PA     BG2 Rotation/Scaling Parameter A (dx)
        2,2,     // 0x4000022     W    BG2PB     BG2 Rotation/Scaling Parameter B (dmx)
        2,2,     // 0x4000024     W    BG2PC     BG2 Rotation/Scaling Parameter C (dy)
        2,2,     // 0x4000026     W    BG2PD     BG2 Rotation/Scaling Parameter D (dmy)
        4,4,4,4, // 0x4000028     W    BG2X      BG2 Reference Point X-Coordinate
        4,4,4,4, // 0x400002C     W    BG2Y      BG2 Reference Point Y-Coordinate
        2,2,     // 0x4000030     W    BG3PA     BG3 Rotation/Scaling Parameter A (dx)
        2,2,     // 0x4000032     W    BG3PB     BG3 Rotation/Scaling Parameter B (dmx)
        2,2,     // 0x4000034     W    BG3PC     BG3 Rotation/Scaling Parameter C (dy)
        2,2,     // 0x4000036     W    BG3PD     BG3 Rotation/Scaling Parameter D (dmy)
        4,4,4,4, // 0x4000038     W    BG3X      BG3 Reference Point X-Coordinate
        4,4,4,4, // 0x400003C     W    BG3Y      BG3 Reference Point Y-Coordinate
        2,2,     // 0x4000040     W    WIN0H     Window 0 Horizontal Dimensions
        2,2,     // 0x4000042     W    WIN1H     Window 1 Horizontal Dimensions
        2,2,     // 0x4000044     W    WIN0V     Window 0 Vertical Dimensions
        2,2,     // 0x4000046     W    WIN1V     Window 1 Vertical Dimensions
        2,2,     // 0x4000048     R/W  WININ     Inside of Window 0 and 1
        2,2,     // 0x400004A     R/W  WINOUT    Inside of OBJ Window & Outside of Windows
        2,2,     // 0x400004C     W    MOSAIC    Mosaic Size
        0,0,
        2,2,     // 0x4000050      R/W  BLDCNT    Color Special Effects Selection
        2,2,     // 0x4000052      R/W  BLDALPHA  Alpha Blending Coefficients
        2,2,     // 0x4000054      W    BLDY      Brightness (Fade-In/Out) Coefficient
        0,0,0,0,0,0,0,0,0,0,

        // Sound registers
        2,2,     // 0x4000060    R/W  SOUND1CNT_L Channel 1 Sweep register       (NR10)
        2,2,     // 0x4000062    R/W  SOUND1CNT_H Channel 1 Duty/Length/Envelope (NR11, NR12)
        2,2,     // 0x4000064    R/W  SOUND1CNT_X Channel 1 Frequency/Control    (NR13, NR14)
        0,0,
        2,2,     // 0x4000068    R/W  SOUND2CNT_L Channel 2 Duty/Length/Envelope (NR21, NR22)
        0,0,
        2,2,     // 0x400006C    R/W  SOUND2CNT_H Channel 2 Frequency/Control    (NR23, NR24)
        0,0,
        2,2,     // 0x4000070    R/W  SOUND3CNT_L Channel 3 Stop/Wave RAM select (NR30)
        2,2,     // 0x4000072    R/W  SOUND3CNT_H Channel 3 Length/Volume        (NR31, NR32)
        2,2,     // 0x4000074    R/W  SOUND3CNT_X Channel 3 Frequency/Control    (NR33, NR34)
        0,0,
        2,2,     // 0x4000078    R/W  SOUND4CNT_L Channel 4 Length/Envelope      (NR41, NR42)
        0,0,
        2,2,     // 0x400007C    R/W  SOUND4CNT_H Channel 4 Frequency/Control    (NR43, NR44)
        0,0,
        2,2,     // 0x4000080    R/W  SOUNDCNT_L  Control Stereo/Volume/Enable   (NR50, NR51)
        2,2,     // 0x4000082    R/W  SOUNDCNT_H  Control Mixing/DMA Control
        2,2,     // 0x4000084    R/W  SOUNDCNT_X  Control Sound on/off           (NR52)
        0,0,
        2,2,     // 0x4000088    BIOS SOUNDBIAS   Sound PWM Control
        0,0,0,0,0,0, // 6 unused bytes
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0x4000090  2x10h R/W  WAVE_RAM  Channel 3 Wave Pattern RAM (2 banks!!)
        4,4,4,4, // 0x40000A0      W    FIFO_A    Channel A FIFO, Data 0-3
        4,4,4,4, // 0x40000A4      W    FIFO_B    Channel B FIFO, Data 0-3
        0,0,0,0,0,0,0,0, // 8 unused bytes

        // DMA transfer channels
        4,4,4,4, // 0x40000B0      W    DMA0SAD   DMA 0 Source Address
        4,4,4,4, // 0x40000B4      W    DMA0DAD   DMA 0 Destination Address
        2,2,     // 0x40000B8      W    DMA0CNT_L DMA 0 Word Count
        2,2,     // 0x40000BA      R/W  DMA0CNT_H DMA 0 Control
        4,4,4,4, // 0x40000BC      W    DMA1SAD   DMA 1 Source Address
        4,4,4,4, // 0x40000C0      W    DMA1DAD   DMA 1 Destination Address
        2,2,     // 0x40000C4      W    DMA1CNT_L DMA 1 Word Count
        2,2,     // 0x40000C6      R/W  DMA1CNT_H DMA 1 Control
        4,4,4,4, // 0x40000C8      W    DMA2SAD   DMA 2 Source Address
        4,4,4,4, // 0x40000CC      W    DMA2DAD   DMA 2 Destination Address
        2,2,     // 0x40000D0      W    DMA2CNT_L DMA 2 Word Count
        2,2,     // 0x40000D2      R/W  DMA2CNT_H DMA 2 Control
        4,4,4,4, // 0x40000D4      W    DMA3SAD   DMA 3 Source Address
        4,4,4,4, // 0x40000D8      W    DMA3DAD   DMA 3 Destination Address
        2,2,     // 0x40000DC      W    DMA3CNT_L DMA 3 Word Count
        2,2,     // 0x40000DE      R/W  DMA3CNT_H DMA 3 Control
        // 32 unused bytes
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,

        // Timer registers
        2,2,     // 0x4000100      R/W  TM0CNT_L  Timer 0 Counter/Reload
        2,2,     // 0x4000102      R/W  TM0CNT_H  Timer 0 Control
        2,2,     // 0x4000104      R/W  TM1CNT_L  Timer 1 Counter/Reload
        2,2,     // 0x4000106      R/W  TM1CNT_H  Timer 1 Control
        2,2,     // 0x4000108      R/W  TM2CNT_L  Timer 2 Counter/Reload
        2,2,     // 0x400010A      R/W  TM2CNT_H  Timer 2 Control
        2,2,     // 0x400010C      R/W  TM3CNT_L  Timer 3 Counter/Reload
        2,2,     // 0x400010E      R/W  TM3CNT_H  Timer 3 Control
        // 16 unused bytes
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,

        // Serial Communication (1)
        // 4,4,4,4, // 0x4000120      R/W  SIODATA32 SIO Data (Normal-32bit Mode; shared with below)
        2,2,     // 0x4000120      R/W  SIOMULTI0 SIO Data 0 (Parent)    (Multi-Player Mode)
        2,2,     // 0x4000122      R/W  SIOMULTI1 SIO Data 1 (1st Child) (Multi-Player Mode)
        2,2,     // 0x4000124      R/W  SIOMULTI2 SIO Data 2 (2nd Child) (Multi-Player Mode)
        2,2,     // 0x4000126      R/W  SIOMULTI3 SIO Data 3 (3rd Child) (Multi-Player Mode)
        2,2,     // 0x4000128      R/W  SIOCNT    SIO Control Register
        2,2,     // 0x400012A      R/W  SIOMLT_SEND SIO Data (Local of MultiPlayer; shared below)
        // 2,2,     // 0x400012A      R/W  SIODATA8  SIO Data (Normal-8bit and UART Mode)
        0,0,0,0,

        // Keypad Input
        2,2,     // 0x4000130      R    KEYINPUT  Key Status
        2,2,     // 0x4000132      R/W  KEYCNT    Key Interrupt Control

        // Serial Communication (2)
        2,2,     // 0x4000134      R/W  RCNT      SIO Mode Select/General Purpose Data
        0,0, // 0x4000136  -    -    IR        Ancient - Infrared Register (Prototypes only) - marked as unused here
        0,0,0,0,0,0,0,0,
        2,2,     // 0x4000140      R/W  JOYCNT    SIO JOY Bus Control
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        4,4,4,4, // 0x4000150      R/W  JOY_RECV  SIO JOY Bus Receive Data
        4,4,4,4, // 0x4000154      R/W  JOY_TRANS SIO JOY Bus Transmit Data
        2,2,     // 0x4000158      R/?  JOYSTAT   SIO JOY Bus Receive Status

        // 166 unused bytes
        // 20 groups of 8 + 6
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,


        // Interrupt, Waitstate, and Power-Down control
        2,2,     // 0x4000200      R/W  IE        Interrupt Enable Register
        2,2,     // 0x4000202      R/W  IF        Interrupt Request Flags / IRQ Acknowledge
        2,2,     // 0x4000204      R/W  WAITCNT   Game Pak Waitstate Control
        0,0,
        2,2,     // 0x4000208      R/W  IME       Interrupt Master Enable Register
        // 246 unused bytes
        // 4 groups of 50 (5 subgroups of 10)
        // 1 group of 40 (4 subgroups of 10)
        // + 6
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,
        1,       // 0x4000300      R/W  POSTFLG   Undocumented - Post Boot Flag
        1,       // 0x4000301      W    HALTCNT   Undocumented - Power Down Control
        // 270 unused bytes
        // 5 groups of 50 (5 subgroups of 10)
        // 1 group of 20 (2 subgroups of 10)
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,

        0, // 0x4000410  ?    ?    ?         Undocumented - Purpose Unknown / Bug ??? 0FFh
        // 0x4000411       -    -         Not used
        // 1007 unused bytes
        // 15 groups of 64 (8 subgroups of 8) = 960
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        // 32
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        // 8
        0,0,0,0,0,0,0,0,
        // 7
        0,0,0,0,0,0,0,

        4,4,4,4 // 0x4000800      R/W  ?         Undocumented - Internal Memory Control (R/W)
        // 0x4000804       -    -         Not used
        // 4,4,4,4, // 0x4xx0800      R/W  ?         Mirrors of 0x4000800 (repeated each 64K)
};

typedef enum ioreg_access {
    U, // Unused
    R, // Readonly
    W, // Writeonly
    RW // Read/Write
} ioreg_access_t;

ioreg_access_t io_register_access[] = {
        // LCD I/O registers
        RW,RW,     // 0x4000000     R/W  DISPCNT   LCD Control
        RW,RW,     // 0x4000002     R/W  -         Undocumented - Green Swap
        RW,RW,     // 0x4000004     R/W  DISPSTAT  General LCD Status (STAT,LYC)
        R,R,     // 0x4000006     R    VCOUNT    Vertical Counter (LY)
        RW,RW,     // 0x4000008     R/W  BG0CNT    BG0 Control
        RW,RW,     // 0x400000A     R/W  BG1CNT    BG1 Control
        RW,RW,     // 0x400000C     R/W  BG2CNT    BG2 Control
        RW,RW,     // 0x400000E     R/W  BG3CNT    BG3 Control
        W,W,     // 0x4000010     W    BG0HOFS   BG0 X-Offset
        W,W,     // 0x4000012     W    BG0VOFS   BG0 Y-Offset
        W,W,     // 0x4000014     W    BG1HOFS   BG1 X-Offset
        W,W,     // 0x4000016     W    BG1VOFS   BG1 Y-Offset
        W,W,     // 0x4000018     W    BG2HOFS   BG2 X-Offset
        W,W,     // 0x400001A     W    BG2VOFS   BG2 Y-Offset
        W,W,     // 0x400001C     W    BG3HOFS   BG3 X-Offset
        W,W,     // 0x400001E     W    BG3VOFS   BG3 Y-Offset
        W,W,     // 0x4000020     W    BG2PA     BG2 Rotation/Scaling Parameter A (dx)
        W,W,     // 0x4000022     W    BG2PB     BG2 Rotation/Scaling Parameter B (dmx)
        W,W,     // 0x4000024     W    BG2PC     BG2 Rotation/Scaling Parameter C (dy)
        W,W,     // 0x4000026     W    BG2PD     BG2 Rotation/Scaling Parameter D (dmy)
        W,W,W,W, // 0x4000028     W    BG2X      BG2 Reference Point X-Coordinate
        W,W,W,W, // 0x400002C     W    BG2Y      BG2 Reference Point Y-Coordinate
        W,W,     // 0x4000030     W    BG3PA     BG3 Rotation/Scaling Parameter A (dx)
        W,W,     // 0x4000032     W    BG3PB     BG3 Rotation/Scaling Parameter B (dmx)
        W,W,     // 0x4000034     W    BG3PC     BG3 Rotation/Scaling Parameter C (dy)
        W,W,     // 0x4000036     W    BG3PD     BG3 Rotation/Scaling Parameter D (dmy)
        W,W,W,W, // 0x4000038     W    BG3X      BG3 Reference Point X-Coordinate
        W,W,W,W, // 0x400003C     W    BG3Y      BG3 Reference Point Y-Coordinate
        W,W,     // 0x4000040     W    WIN0H     Window 0 Horizontal Dimensions
        W,W,     // 0x4000042     W    WIN1H     Window 1 Horizontal Dimensions
        W,W,     // 0x4000044     W    WIN0V     Window 0 Vertical Dimensions
        W,W,     // 0x4000046     W    WIN1V     Window 1 Vertical Dimensions
        RW,RW,     // 0x4000048     R/W  WININ     Inside of Window 0 and 1
        RW,RW,     // 0x400004A     R/W  WINOUT    Inside of OBJ Window & Outside of Windows
        W,W,     // 0x400004C     W    MOSAIC    Mosaic Size
        0,0,
        RW,RW,     // 0x4000050      R/W  BLDCNT    Color Special Effects Selection
        RW,RW,     // 0x4000052      R/W  BLDALPHA  Alpha Blending Coefficients
        W,W,     // 0x4000054      W    BLDY      Brightness (Fade-In/Out) Coefficient
        0,0,0,0,0,0,0,0,0,0,

        // Sound registers
        RW,RW,     // 0x4000060    R/W  SOUND1CNT_L Channel 1 Sweep register       (NR10)
        RW,RW,     // 0x4000062    R/W  SOUND1CNT_H Channel 1 Duty/Length/Envelope (NR11, NR12)
        RW,RW,     // 0x4000064    R/W  SOUND1CNT_X Channel 1 Frequency/Control    (NR13, NR14)
        0,0,
        RW,RW,     // 0x4000068    R/W  SOUND2CNT_L Channel 2 Duty/Length/Envelope (NR21, NR22)
        0,0,
        RW,RW,     // 0x400006C    R/W  SOUND2CNT_H Channel 2 Frequency/Control    (NR23, NR24)
        0,0,
        RW,RW,     // 0x4000070    R/W  SOUND3CNT_L Channel 3 Stop/Wave RAM select (NR30)
        RW,RW,     // 0x4000072    R/W  SOUND3CNT_H Channel 3 Length/Volume        (NR31, NR32)
        RW,RW,     // 0x4000074    R/W  SOUND3CNT_X Channel 3 Frequency/Control    (NR33, NR34)
        0,0,
        RW,RW,     // 0x4000078    R/W  SOUND4CNT_L Channel 4 Length/Envelope      (NR41, NR42)
        0,0,
        RW,RW,     // 0x400007C    R/W  SOUND4CNT_H Channel 4 Frequency/Control    (NR43, NR44)
        0,0,
        RW,RW,     // 0x4000080    R/W  SOUNDCNT_L  Control Stereo/Volume/Enable   (NR50, NR51)
        RW,RW,     // 0x4000082    R/W  SOUNDCNT_H  Control Mixing/DMA Control
        RW,RW,     // 0x4000084    R/W  SOUNDCNT_X  Control Sound on/off           (NR52)
        0,0,
        RW,RW,     // 0x4000088    BIOS SOUNDBIAS   Sound PWM Control
        0,0,0,0,0,0, // 6 unused bytes
        RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW,RW, // 0x4000090  2x10h R/W  WAVE_RAM  Channel 3 Wave Pattern RAM (2 banks!!)
        W,W,W,W, // 0x40000A0      W    FIFO_A    Channel A FIFO, Data 0-3
        W,W,W,W, // 0x40000A4      W    FIFO_B    Channel B FIFO, Data 0-3
        0,0,0,0,0,0,0,0, // 8 unused bytes

        // DMA transfer channels
        W,W,W,W, // 0x40000B0      W    DMA0SAD   DMA 0 Source Address
        W,W,W,W, // 0x40000B4      W    DMA0DAD   DMA 0 Destination Address
        W,W,     // 0x40000B8      W    DMA0CNT_L DMA 0 Word Count
        RW,RW,     // 0x40000BA      R/W  DMA0CNT_H DMA 0 Control
        W,W,W,W, // 0x40000BC      W    DMA1SAD   DMA 1 Source Address
        W,W,W,W, // 0x40000C0      W    DMA1DAD   DMA 1 Destination Address
        W,W,     // 0x40000C4      W    DMA1CNT_L DMA 1 Word Count
        RW,RW,     // 0x40000C6      R/W  DMA1CNT_H DMA 1 Control
        W,W,W,W, // 0x40000C8      W    DMA2SAD   DMA 2 Source Address
        W,W,W,W, // 0x40000CC      W    DMA2DAD   DMA 2 Destination Address
        W,W,     // 0x40000D0      W    DMA2CNT_L DMA 2 Word Count
        RW,RW,     // 0x40000D2      R/W  DMA2CNT_H DMA 2 Control
        W,W,W,W, // 0x40000D4      W    DMA3SAD   DMA 3 Source Address
        W,W,W,W, // 0x40000D8      W    DMA3DAD   DMA 3 Destination Address
        W,W,     // 0x40000DC      W    DMA3CNT_L DMA 3 Word Count
        RW,RW,     // 0x40000DE      R/W  DMA3CNT_H DMA 3 Control
        // 32 unused bytes
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,

        // Timer registers
        RW,RW,     // 0x4000100      R/W  TM0CNT_L  Timer 0 Counter/Reload
        RW,RW,     // 0x4000102      R/W  TM0CNT_H  Timer 0 Control
        RW,RW,     // 0x4000104      R/W  TM1CNT_L  Timer 1 Counter/Reload
        RW,RW,     // 0x4000106      R/W  TM1CNT_H  Timer 1 Control
        RW,RW,     // 0x4000108      R/W  TM2CNT_L  Timer 2 Counter/Reload
        RW,RW,     // 0x400010A      R/W  TM2CNT_H  Timer 2 Control
        RW,RW,     // 0x400010C      R/W  TM3CNT_L  Timer 3 Counter/Reload
        RW,RW,     // 0x400010E      R/W  TM3CNT_H  Timer 3 Control
        // 16 unused bytes
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,

        // Serial Communication (1)
        // 4,4,4,4, // 0x4000120      R/W  SIODATA32 SIO Data (Normal-32bit Mode; shared with below)
        RW,RW,     // 0x4000120      R/W  SIOMULTI0 SIO Data 0 (Parent)    (Multi-Player Mode)
        RW,RW,     // 0x4000122      R/W  SIOMULTI1 SIO Data 1 (1st Child) (Multi-Player Mode)
        RW,RW,     // 0x4000124      R/W  SIOMULTI2 SIO Data 2 (2nd Child) (Multi-Player Mode)
        RW,RW,     // 0x4000126      R/W  SIOMULTI3 SIO Data 3 (3rd Child) (Multi-Player Mode)
        RW,RW,     // 0x4000128      R/W  SIOCNT    SIO Control Register
        RW,RW,     // 0x400012A      R/W  SIOMLT_SEND SIO Data (Local of MultiPlayer; shared below)
        // 2,2,     // 0x400012A      R/W  SIODATA8  SIO Data (Normal-8bit and UART Mode)
        0,0,0,0,

        // Keypad Input
        R,R,     // 0x4000130      R    KEYINPUT  Key Status
        RW,RW,     // 0x4000132      R/W  KEYCNT    Key Interrupt Control

        // Serial Communication (2)
        RW,RW,     // 0x4000134      R/W  RCNT      SIO Mode Select/General Purpose Data
        0,0, // 0x4000136  -    -    IR        Ancient - Infrared Register (Prototypes only) - marked as unused here
        0,0,0,0,0,0,0,0,
        RW,RW,     // 0x4000140      R/W  JOYCNT    SIO JOY Bus Control
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        RW,RW,RW,RW, // 0x4000150      R/W  JOY_RECV  SIO JOY Bus Receive Data
        RW,RW,RW,RW, // 0x4000154      R/W  JOY_TRANS SIO JOY Bus Transmit Data
        RW,RW,     // 0x4000158      R/?  JOYSTAT   SIO JOY Bus Receive Status

        // 166 unused bytes
        // 20 groups of 8 + 6
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,


        // Interrupt, Waitstate, and Power-Down control
        RW,RW,     // 0x4000200      R/W  IE        Interrupt Enable Register
        RW,RW,     // 0x4000202      R/W  IF        Interrupt Request Flags / IRQ Acknowledge
        RW,RW,     // 0x4000204      R/W  WAITCNT   Game Pak Waitstate Control
        0,0,
        RW,RW,     // 0x4000208      R/W  IME       Interrupt Master Enable Register
        // 246 unused bytes
        // 4 groups of 50 (5 subgroups of 10)
        // 1 group of 40 (4 subgroups of 10)
        // + 6
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,
        RW,       // 0x4000300      R/W  POSTFLG   Undocumented - Post Boot Flag
        W,       // 0x4000301      W    HALTCNT   Undocumented - Power Down Control
        // 270 unused bytes
        // 5 groups of 50 (5 subgroups of 10)
        // 1 group of 20 (2 subgroups of 10)
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,

        0, // 0x4000410  ?    ?    ?         Undocumented - Purpose Unknown / Bug ??? 0FFh
        // 0x4000411       -    -         Not used
        // 1007 unused bytes
        // 15 groups of 64 (8 subgroups of 8) = 960
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        // 32
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        // 8
        0,0,0,0,0,0,0,0,
        // 7
        0,0,0,0,0,0,0,

        RW,RW,RW,RW // 0x4000800      R/W  ?         Undocumented - Internal Memory Control (R/W)
        // 0x4000804       -    -         Not used
        // 4,4,4,4, // 0x4xx0800      R/W  ?         Mirrors of 0x4000800 (repeated each 64K)
};

byte get_ioreg_size_for_addr(word address) {
    word mirrored = mirror_ioreg(address);
    if (mirrored >= 0x4000804) {
        return 0;
    }
    else {
        return io_register_sizes[ioreg_index(mirrored)];
    }
}

bool is_ioreg_readable(word address) {
    word mirrored = mirror_ioreg(address);
    if (mirrored >= 0x4000804) {
        return false;
    }
    else {
        ioreg_access_t access = io_register_access[ioreg_index(mirrored)];
        return access == R || access == RW;
    }
}


bool is_ioreg_writable(word address) {
    word mirrored = mirror_ioreg(address);
    if (mirrored >= 0x4000804) {
        return false;
    }
    else {
        ioreg_access_t access = io_register_access[ioreg_index(mirrored)];
        return access == W || access == RW;
    }
}


#endif //GBA_IOREG_UTIL_H
