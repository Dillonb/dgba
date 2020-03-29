//
// Created by dillon on 3/29/20.
//

#include "dma.h"
#include "../common/log.h"

int dma(int n, DMACNTH_t* cnth, DMAINT_t* dmaint, word sad, word dad, word wc, word max_wc) {
    int dma_cycles = 0;
    if (cnth->dma_enable) {

        unimplemented(cnth->game_pak_drq_dma3_only, "Game pak DRQ")
        if (cnth->dma_start_time != 0) {
            if (n != 1 && n != 2) {
                logfatal("Non-immediate DMA start time (grr)")
            } else {
                if (cnth->dma_start_time == Special) {
                    return 0; // Ignored
                } else {
                    logfatal("Non-immediate Non-Special DMA start time (grr)")
                }
            }
        }
        // When newly enabled, reload everything
        if (!dmaint->previously_enabled) {
            dmaint->previously_enabled = true;
            dmaint->current_source_address = sad;
            dmaint->current_dest_address = dad;


            dmaint->remaining = wc;
            if (dmaint->remaining == 0) {
                dmaint->remaining = max_wc;
            }

            logwarn("DMA%d initialized: 0x%08X => 0x%08X * %d width: %s src: %d, dest: %d",
                    n, sad, dad, dmaint->remaining, cnth->dma_transfer_type ? "32b" : "16b",
                    cnth->source_addr_control, cnth->dest_addr_control)
            logwarn("DMA%dCNT_H set to 0x%08X", n, cnth->raw);
        }

        if (dmaint->remaining > 0) {
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
                switch (cnth->dest_addr_control) {
                    case 0: dmaint->current_dest_address += sizeof(half); break;
                    case 1: dmaint->current_dest_address -= sizeof(half); break;
                    case 2: break; // No change
                    default: logfatal("Unimplemented dest address control type: %d", cnth->dest_addr_control)
                }
                //logwarn("DMA%d: pc: 0x%08X transferred 0x%04X from 0x%08X to 0x%08X", n, cpu->pc, value, source_address, dest_address)
            } else { // 32 bits
                word source_address = dmaint->current_source_address;
                word value = gba_read_word(source_address);
                dma_cycles++; // TODO real mem access time
                switch (cnth->source_addr_control) {
                    case 0: dmaint->current_source_address += sizeof(word); break;
                    case 1: dmaint->current_source_address -= sizeof(word); break;
                    case 2: break; // No change
                    default: logfatal("Unimplemented source address control type: %d", cnth->source_addr_control)
                }

                word dest_address = dmaint->current_dest_address;
                gba_write_word(dest_address, value);
                dma_cycles++; // TODO real mem access time
                switch (cnth->dest_addr_control) {
                    case 0: dmaint->current_dest_address += sizeof(word); break;
                    case 1: dmaint->current_dest_address -= sizeof(word); break;
                    case 2: break; // No change
                    default: logfatal("Unimplemented dest address control type: %d", cnth->dest_addr_control)
                }

                //logwarn("DMA%d: pc: 0x%08X, transferred 0x%08X from 0x%08X to 0x%08X", n, cpu->pc, value, source_address, dest_address)
            }
            dmaint->remaining--;
        } else {
            unimplemented(cnth->irq_on_end_of_wc, "IRQ on end of DMA. I mean, this shouldn't be hard")
            cnth->dma_enable = cnth->dma_repeat;
            dmaint->previously_enabled = false;
            dma_cycles++;
        }
    }
    return dma_cycles;
}
