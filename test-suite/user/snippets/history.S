#define MAX_HISTORY_SIZE 512


.intel_syntax noprefix

.section .note.GNU-stack,"",@progbits
.section .text

# Both F+R function (user-space) and kernel module calls enter_jit_chain.
# It is the start of each branch path (caller0, caller1 and evicting branches)
.global enter_jit_chain
enter_jit_chain:
    # In case of HISTORY_NONE, we take this branch for history_idx = 1
    # to force a distinguished history
    cmp     r8, 0x0
    je      $+2
    # call underflow_rsb
    mov     rax, rdi    # jit entry
    mov     r11, rsi    # caller
    mov     rdi, rdx    # target

    mov     rsi, r9     # history array if fill_bhb_jmp

    # F+R buffers
    # We push the buffers to stack and we pop them right before
    # the victim branch to minimize FP due to other speculations.
    # (we only do this for main training path)
    push    QWORD PTR [rcx + 8 * 0]
    push    QWORD PTR [rcx + 8 * 1]
    push    QWORD PTR [rcx + 8 * 2]
    # F+R buffers EXTRA training path
    mov     r8, QWORD PTR [rcx + 8 * 3]
    mov     r9, QWORD PTR [rcx + 8 * 4]
    mov     r10, QWORD PTR [rcx + 8 * 5]

    # We set carry flag, in case we use conditional branches in jump chain
    stc
    # We either jump directly to the caller (In case of HISTORY_NONE)
    # or to one of the history chains
    mfence
    jmp     rax
    int3

.global fill_bhb_jmp
.align 4096
fill_bhb_jmp:
    # Perform conditional branches to fill the BHB
    .rept MAX_HISTORY_SIZE
        movzx   rax, BYTE PTR [rsi]
        inc     rsi
        cmp     rax, 1
        je      1f
        1:
    .endr
    stc
    # Now we jump to the caller
    mfence
    jmp     r11
    int3


# This is the exit of the history jump chain
.global jit_exit_start
.global jit_exit_end
jit_exit_start:
     # Now we jump to the caller
    mfence
    jmp     r11
    int3
    int3
jit_exit_end:


.global underflow_rsb
underflow_rsb:
    .rept 16
        call 1f
        ret
        1:
    .endr
    ret
