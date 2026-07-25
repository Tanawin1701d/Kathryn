# tcr01 — first light: ALU imm/reg, branch, la (auipc+addi), sd/ld/lw roundtrip.
#include "pass_fail.h"

.section .text.init
.globl _start
_start:
    li   a0, 2
    addi a0, a0, 3
    li   a1, 5
    bne  a0, a1, fail

    la   t0, scratch
    li   t1, 0x1122334455667788
    sd   t1, 0(t0)
    ld   t2, 0(t0)
    bne  t1, t2, fail

    lw   t3, 0(t0)              # sign-extended low word
    li   t4, 0x55667788
    bne  t3, t4, fail

    lhu  t3, 0(t0)              # zero-extended halfword
    li   t4, 0x7788
    bne  t3, t4, fail

    lb   t3, 6(t0)              # byte at offset 6 = 0x22, sign-extended
    li   t4, 0x22
    bne  t3, t4, fail

    PASS

.data
scratch: .dword 0

TOHOST_DATA
