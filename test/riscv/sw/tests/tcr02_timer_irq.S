# tcr02 — CLINT timer interrupt end to end: program mtimecmp, enable MTIE+MIE,
# take the interrupt (mcause = IRQ|7), silence the timer, mret back, verify.
#include "pass_fail.h"

#define CLINT_MTIME    0x200BFF8
#define CLINT_MTIMECMP 0x2004000

.section .text.init
.globl _start
_start:
    la   t0, handler
    csrw mtvec, t0

    li   t1, CLINT_MTIME
    ld   t2, 0(t1)
    addi t2, t2, 500
    li   t3, CLINT_MTIMECMP
    sd   t2, 0(t3)

    li   t4, 0x80                 # mie.MTIE
    csrw mie, t4
    csrsi mstatus, 0x8            # mstatus.MIE

    li   a0, 0
1:  beqz a0, 1b                   # spin until the handler fires
    li   t5, 42
    bne  a0, t5, fail
    PASS

.align 2
handler:
    csrr t6, mcause
    li   t5, 1
    slli t5, t5, 63
    ori  t5, t5, 7                # expect interrupt bit | MTI (7)
    bne  t6, t5, fail
    li   t3, CLINT_MTIMECMP       # silence the timer
    li   t2, -1
    sd   t2, 0(t3)
    li   a0, 42
    mret

TOHOST_DATA
