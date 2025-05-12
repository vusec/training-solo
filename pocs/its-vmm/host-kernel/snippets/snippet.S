.intel_syntax noprefix

.section .text

.macro ANNOTATE_RETPOLINE_SAFE
.Lhere_\@:
	.pushsection .discard.retpoline_safe
	.long .Lhere_\@
	.popsection
.endm

.macro ANNOTATE_INTRA_FUNCTION_CALL
.Lhere_\@:
	.pushsection .discard.intra_function_calls
	.long .Lhere_\@
	.popsection
.endm

.macro REACHABLE
.Lhere_\@:
	.pushsection .discard.reachable
	.long	.Lhere_\@
	.popsection
.endm



.global a_target0
.align 64
a_target0:
    .rept 64
        nop
    .endr
    ANNOTATE_RETPOLINE_SAFE
    ret
    int3

.global a_target1
.align 64
a_target1:
    .rept 64
        nop
    .endr
    ANNOTATE_RETPOLINE_SAFE
    ret
    int3


.align 4096
    .rept 64
        int3
    .endr

# This will be at PC[5:0] 0x0
.global caller1
caller1:
    lfence
    ANNOTATE_RETPOLINE_SAFE
.global its_victim_ind_branch
its_victim_ind_branch:
    jmp    QWORD PTR [rdi]
.global its_victim_ind_branch_end
its_victim_ind_branch_end:
    int3
    int3

.rept 128
    int3
.endr

.align 64
.global its_speculation_target
its_speculation_target:
    mov r8, QWORD PTR[rdx]
    mov rax, QWORD PTR[rsi + r8]
    int3
    int3
