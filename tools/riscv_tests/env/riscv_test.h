// Minimal CSR-free test environment for riscv-tests, used until the core grows
// Zicsr (M2). Replaces the stock `p` env: PASS/FAIL talk straight to the HTIF
// tohost word the simharness watches; TESTNUM (gp) reports the failing case.

#ifndef _ENV_KATHRYN_TEST_H
#define _ENV_KATHRYN_TEST_H

#define RVTEST_RV64U
#define RVTEST_RV64M
#define RVTEST_RV64A

#define TESTNUM gp

#define RVTEST_CODE_BEGIN \
    .section .text.init;  \
    .globl _start;        \
_start:

#define RVTEST_CODE_END

#define RVTEST_PASS       \
    li   a0, 1;           \
    la   a1, tohost;      \
    sd   a0, 0(a1);       \
1:  j    1b

#define RVTEST_FAIL       \
    sll  a0, TESTNUM, 1;  \
    ori  a0, a0, 1;       \
    la   a1, tohost;      \
    sd   a0, 0(a1);       \
1:  j    1b

#define RVTEST_DATA_BEGIN \
    .data;                \
    .align 4;             \
    .section .tohost, "aw"; \
    .globl tohost;        \
    .align 3;             \
tohost: .dword 0;         \
    .globl fromhost;      \
fromhost: .dword 0;       \
    .data

#define RVTEST_DATA_END

#endif
