add_library(arm7tdmi
        arm7tdmi.c arm7tdmi.h
        shifts.c shifts.h
        sign_extension.c sign_extension.h
        software_interrupt.c software_interrupt.h

        arm_instr/arm_instr.c arm_instr/arm_instr.h
        arm_instr/data_processing.c arm_instr/data_processing.h
        arm_instr/branch.c arm_instr/branch.h
        arm_instr/single_data_transfer.c arm_instr/single_data_transfer.h
        arm_instr/block_data_transfer.c arm_instr/block_data_transfer.h
        arm_instr/status_transfer.c arm_instr/status_transfer.h
        arm_instr/halfword_data_transfer.c arm_instr/halfword_data_transfer.h
        arm_instr/multiply.c arm_instr/multiply.h
        arm_instr/single_data_swap.c arm_instr/single_data_swap.h
        arm_instr/arm_software_interrupt.c arm_instr/arm_software_interrupt.h

        thumb_instr/thumb_instr.c thumb_instr/thumb_instr.h
        thumb_instr/immediate_operations.c thumb_instr/immediate_operations.h
        thumb_instr/high_register_operations.c thumb_instr/high_register_operations.h
        thumb_instr/load_address.c thumb_instr/load_address.h
        thumb_instr/move_shifted_register.c thumb_instr/move_shifted_register.h
        thumb_instr/alu_operations.c thumb_instr/alu_operations.h
        thumb_instr/load_store_halfword.c thumb_instr/load_store_halfword.h
        thumb_instr/pc_relative_load.c thumb_instr/pc_relative_load.h
        thumb_instr/load_store.c thumb_instr/load_store.h
        thumb_instr/sp_relative_load_store.c thumb_instr/sp_relative_load_store.h
        thumb_instr/add_offset_to_stack_pointer.c thumb_instr/add_offset_to_stack_pointer.h
        thumb_instr/push_pop_registers.c thumb_instr/push_pop_registers.h
        thumb_instr/multiple_load_store.c thumb_instr/multiple_load_store.h
        thumb_instr/conditional_branch.c thumb_instr/conditional_branch.h
        thumb_instr/thumb_software_interrupt.c thumb_instr/thumb_software_interrupt.h
        thumb_instr/unconditional_branch.c thumb_instr/unconditional_branch.h
        thumb_instr/long_branch_link.c thumb_instr/long_branch_link.h
        thumb_instr/add_subtract.c thumb_instr/add_subtract.h)

target_link_libraries(arm7tdmi common)
