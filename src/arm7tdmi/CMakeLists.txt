add_library(arm7tdmi
        arm7tdmi.c arm7tdmi.h
        arm_instr.h
        data_processing.c data_processing.h
        branch.c branch.h
        single_data_transfer.c single_data_transfer.h
        shifts.c shifts.h
        )

target_link_libraries(arm7tdmi common)
