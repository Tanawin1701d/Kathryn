# tcr04 — RAM pattern sweep with mixed access sizes: write a page of computed
# 64-bit patterns, read back as dwords/words/halves/bytes and re-verify.
#include "pass_fail.h"

#define BASE  0x80010000
#define WORDS 512

.section .text.init
.globl _start
_start:
    li   t0, BASE
    li   t1, WORDS
    li   t2, 0x9E3779B97F4A7C15    # golden-ratio stride
    li   t3, 0
fill:
    mul  t4, t3, t2
    xor  t4, t4, t3
    slli t5, t3, 3
    add  t5, t5, t0
    sd   t4, 0(t5)
    addi t3, t3, 1
    bne  t3, t1, fill

    li   t3, 0
check:
    mul  t4, t3, t2
    xor  t4, t4, t3
    slli t5, t3, 3
    add  t5, t5, t0
    ld   t6, 0(t5)                 # dword
    bne  t6, t4, fail
    lwu  t6, 4(t5)                 # high word, zero-extended
    srli a2, t4, 32
    bne  t6, a2, fail
    lhu  t6, 2(t5)                 # half at offset 2
    srli a2, t4, 16
    li   a3, 0xFFFF
    and  a2, a2, a3
    bne  t6, a2, fail
    lbu  t6, 7(t5)                 # top byte
    srli a2, t4, 56
    bne  t6, a2, fail
    addi t3, t3, 1
    bne  t3, t1, check
    PASS

TOHOST_DATA
