#include "thumb_software_interrupt.h"
#include "../../common/log.h"

static const char* SWI_NAMES[] = {
        "SoftReset",
        "RegisterRamReset",
        "Halt",
        "Stop/Sleep",
        "IntrWait",
        "VBlankIntrWait",
        "Div",
        "DivArm",
        "Sqrt",
        "ArcTan",
        "ArcTan2",
        "CpuSet",
        "CpuFastSet",
        "GetBiosChecksum",
        "BgAffineSet",
        "ObjAffineSet",
        "BitUnPack",
        "LZ77UnCompReadNormalWrite8bit",
        "LZ77UnCompReadNormalWrite16bit",
        "LZ77UnCompReadByCallbackWrite8bit",
        "LZ77UnCompReadByCallbackWrite16bit",
        "LZ77UnCompReadByCallbackWrite16bit (same as above)",
        "HuffUnCompReadNormal",
        "HuffUnCompReadByCallback",
        "RLUnCompReadNormalWrite8bit",
        "RLUnCompReadNormalWrite16bit",
        "RLUnCompReadByCallbackWrite16bit",
        "Diff8bitUnFilterWrite8bit",
        "Diff8bitUnFilterWrite16bit",
        "Diff16bitUnFilter",
        "Sound (and Multiboot/HardReset/CustomHalt)",
        "SoundBias",
        "SoundDriverInit",
        "SoundDriverMode",
        "SoundDriverMain",
        "SoundDriverVSync",
        "SoundChannelClear",
        "MidiKey2Freq",
        "SoundWhatever0",
        "SoundWhatever1",
        "SoundWhatever2",
        "SoundWhatever3",
        "SoundWhatever4",
        "MultiBoot",
        "HardReset",
        "CustomHalt",
        "SoundDriverVSyncOff",
        "SoundDriverVSyncOn",
        "SoundGetJumpList",
        "New NDS Functions",
        "WaitByLoop",
        "GetCRC16",
        "IsDebugger",
        "GetSineTable",
        "GetPitchTable (DSi7: bugged)",
        "GetVolumeTable",
        "GetBootProcs (DSi7: only 1 proc)",
        "CustomPost",
        "New DSi Functions (RSA/SHA1)",
        "RSA_Init_crypto_heap",
        "RSA_Decrypt",
        "RSA_Decrypt_Unpad",
        "RSA_Decrypt_Unpad_OpenPGP_SHA1",
        "SHA1_Init",
        "SHA1_Update",
        "SHA1_Finish",
        "SHA1_Init_update_fin",
        "SHA1_Compare_20_bytes",
        "SHA1_Random_maybe",
        "Invalid Functions",
        "Crash (SWI xxh..FFh do jump to garbage addresses)",
        "Jump to 0   (on any SWI numbers not listed above)",
        "No function (ignored)",
        "No function (ignored)",
        "Mirror      (SWI 40h..FFh mirror to 00h..3Fh)",
        "Hang        (on any SWI numbers not listed above)",
};

void thumb_software_interrupt(arm7tdmi_t* state, thumb_software_interrupt_t* instr) {
    logwarn("SWI: 0x%X - %s", instr->value, SWI_NAMES[instr->value])
    status_register_t cpsr = state->cpsr;
    state->cpsr.mode = MODE_SUPERVISOR;
    set_spsr(state, cpsr.raw);

    state->lr_svc = state->pc - 2;

    state->cpsr.thumb = 0;
    state->cpsr.disable_irq = 1;

    set_pc(state, 0x8); // SVC handler
}
