add_library(arm7tdmi
        arm7tdmi.c arm7tdmi.h
        shifts.c shifts.h

        arm_instr/arm_instr.c arm_instr/arm_instr.h
        arm_instr/data_processing.c arm_instr/data_processing.h
        arm_instr/branch.c arm_instr/branch.h
        arm_instr/single_data_transfer.c arm_instr/single_data_transfer.h
        arm_instr/block_data_transfer.c arm_instr/block_data_transfer.h
        arm_instr/status_transfer.c arm_instr/status_transfer.h
        arm_instr/halfword_data_transfer.c arm_instr/halfword_data_transfer.h

        thumb_instr/thumb_instr.c thumb_instr/thumb_instr.h
        thumb_instr/immediate_operations.c thumb_instr/immediate_operations.h
        thumb_instr/high_register_operations.c thumb_instr/high_register_operations.h
        thumb_instr/load_address.c thumb_instr/load_address.h)

target_link_libraries(arm7tdmi common)
