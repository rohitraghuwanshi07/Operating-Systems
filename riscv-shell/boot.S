.section .text.init
.global _start

_start:
    la sp, stack_top   # set up stack pointer
    call main          # jump to C code
loop:
    j loop              # halt if main() ever returns

.section .bss
.align 4
stack_bottom:
    .skip 4096 * 4       # 16KB stack
stack_top:
