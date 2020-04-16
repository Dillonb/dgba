#include "dma.h"
#include "../common/log.h"
#include "../gba_system.h"
#include "ioreg_names.h"

static dma_start_time_t dma_trigger = Immediately;

static const char* dma_triggers[] = {
        "Immediate",
        "VBlank",
        "HBlank",
        "Refresh"
};

void dma_start_trigger(dma_start_time_t trigger) {
    dma_trigger = trigger;
}

int dma(int n, DMACNTH_t* cnth, DMAINT_t* dmaint, word sad, word dad, word wc, word max_wc) {
    int dma_cycles = 0;
    bool is_sound_dma = false;
    if (cnth->dma_enable) {
        unimplemented(cnth->game_pak_drq_dma3_only, "Game pak DRQ")
        if (cnth->dma_start_time != Immediately && cnth->dma_start_time != dma_trigger) {
            if (cnth->dma_start_time == Special) {
                if (n == 0) {
                    logfatal("Special start time for DMA0 is invalid!")
                }
                else if (n == 1 || n == 2) {
                    int fifo_index = 0;
                    is_sound_dma = true;

                    if ((dad & 0xFFF) == IO_FIFO_A) {
                        fifo_index = 0;
                    } else if ((dad & 0xFFF) == IO_FIFO_B) {
                        fifo_index = 1;
                    } else {
                        logfatal("Sound DMA to non-FIFO register")
                    }

                    int fifo_size = apu->fifo[fifo_index].write_index - apu->fifo[fifo_index].read_index;

                    // If the FIFO is more than half full, don't bother running the DMA yet
                    if (fifo_size > (SOUND_FIFO_SIZE / 2)) {
                        return 0;
                    }
                }
            } else {
                return 0;
            }
        }

        // When newly enabled, reload everything
        if (!dmaint->previously_enabled) {
            dmaint->previously_enabled = true;
            dmaint->current_source_address = sad;
            dmaint->current_dest_address = dad;
        } else if (cnth->dest_addr_control == 3) {
            dmaint->previously_enabled = true;
            dmaint->current_dest_address = dad;
        }

        dmaint->remaining = wc;
        if (dmaint->remaining == 0) {
            dmaint->remaining = max_wc;
        }

        logwarn("DMA%d triggered: at %s - 0x%08X => 0x%08X * %d width: %s src: %d, dest: %d",
                n, dma_triggers[cnth->dma_start_time], sad, dad, dmaint->remaining, cnth->dma_transfer_type ? "32b" : "16b",
                cnth->source_addr_control, cnth->dest_addr_control)
        logwarn("DMA%dCNT_H set to 0x%08X", n, cnth->raw);

        while (dmaint->remaining > 0) {
            if (cnth->dma_transfer_type == 0) {// 16 bits
                word source_address = dmaint->current_source_address;
                half value = gba_read_half(source_address);
                dma_cycles++; // TODO real mem access time
                switch (cnth->source_addr_control) {
                    case 0: dmaint->current_source_address += sizeof(half); break;
                    case 1: dmaint->current_source_address -= sizeof(half); break;
                    case 2: break; // No change
                    default: logfatal("Unimplemented source address control type: %d", cnth->source_addr_control)
                }

                word dest_address = dmaint->current_dest_address;
                gba_write_half(dest_address, value);
                dma_cycles++;
                if (!is_sound_dma) {
                    switch (cnth->dest_addr_control) {
                        case 0: dmaint->current_dest_address += sizeof(half); break;
                        case 1: dmaint->current_dest_address -= sizeof(half); break;
                        case 2: break; // No change
                        case 3: dmaint->current_dest_address += sizeof(half); break; // Reset after transfer on repeat
                        default: logfatal("Unimplemented dest address control type: %d", cnth->dest_addr_control)
                    }
                }
                logwarn("DMA%d: transferred 0x%04X from 0x%08X to 0x%08X", n, value, source_address, dest_address)
            } else { // 32 bits
                word source_address = dmaint->current_source_address;
                word value = gba_read_word(source_address);
                dma_cycles++; // TODO real mem access time
                switch (cnth->source_addr_control) {
                    case 0: dmaint->current_source_address += sizeof(word); break;
                    case 1: dmaint->current_source_address -= sizeof(word); break;
                    case 2: break; // No change
                    case 3: dmaint->current_source_address += sizeof(word); break; // Reset after transfer on repeat
                    default: logfatal("Unimplemented source address control type: %d", cnth->source_addr_control)
                }

                word dest_address = dmaint->current_dest_address;
                gba_write_word(dest_address, value);
                dma_cycles++; // TODO real mem access time
                if (!is_sound_dma) { // Only inc when not a sound DMA
                    switch (cnth->dest_addr_control) {
                        case 0: dmaint->current_dest_address += sizeof(word); break;
                        case 1: dmaint->current_dest_address -= sizeof(word); break;
                        case 2: break; // No change
                        case 3: dmaint->current_dest_address += sizeof(word); break; // Reset after transfer on repeat
                        default: logfatal("Unimplemented dest address control type: %d", cnth->dest_addr_control)
                    }
                }

                logwarn("DMA%d: transferred 0x%08X from 0x%08X to 0x%08X", n, value, source_address, dest_address)
            }
            dmaint->remaining--;
        }
        logwarn("DMA%d finished", n)
        if (cnth->irq_on_end_of_wc) {
            switch (n) {
                case 0:
                    request_interrupt(IRQ_DMA0);
                    break;
                case 1:
                    request_interrupt(IRQ_DMA1);
                    break;
                case 2:
                    request_interrupt(IRQ_DMA2);
                    break;
                case 3:
                    request_interrupt(IRQ_DMA3);
                    break;
            }
        }
        cnth->dma_enable = cnth->dma_repeat;
        dmaint->previously_enabled = cnth->dma_enable;
    }
    return dma_cycles;
}

void dma_done_hook() {
    dma_trigger = Immediately;
}