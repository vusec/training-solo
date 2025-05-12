.intel_syntax noprefix

.section .text

.global target_start
.global target_entry
.global target_patch
.global target_end
target_start:
    int3
    int3
target_entry:
target_patch:
    # gadget dependent fr_buf
    .BYTE 0x0f, 0x1f, 0x00
    .BYTE 0x0f, 0x1f, 0x00
    .BYTE 0x90
    lfence
    ret
    int3
    int3
target_end:

.global caller_start
.global caller_entry
.global caller_patch
.global caller_end
caller_start:
    int3
    int3
caller_entry:
    .rept 64 # entry offset
        nop
    .endr
    lfence
    pop rcx # fr_buf target0
    pop rdx # fr_buf target_near
    pop rsi # fr_buf target_short
    .BYTE 0x0f, 0x1f, 0x00
caller_patch:
    .BYTE 0x90, 0x90
    .BYTE 0x90
    jmp [rdi]
    int3
    int3
caller_end:

.global ret_target_start
.global ret_target_entry
.global ret_target_end
ret_target_start:
    int3
    int3
ret_target_entry:
    lfence
    ret
    int3
    int3
ret_target_end:
