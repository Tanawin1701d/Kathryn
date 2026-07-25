// Minimal M-mode test environment for riscv-tests on the Kathryn core.
// Mirrors the stock `p` env's contract that rv64mi tests rely on: a trap
// vector is installed at start and forwards to a test-defined (weak)
// `mtvec_handler` when present; otherwise any trap fails the test. PASS/FAIL
// talk straight to the HTIF tohost word the simharness watches; TESTNUM (gp)
// reports the failing case. No pmp/satp/medeleg touch-and-catch preamble —
// the core has none of them and tests here never enter S/U mode.

#ifndef _ENV_KATHRYN_TEST_H
#define _ENV_KATHRYN_TEST_H

#include "../riscv-tests/env/encoding.h"

#define RVTEST_RV64U
#define RVTEST_RV64M
#define RVTEST_RV64A

#define TESTNUM gp

#define RVTEST_CODE_BEGIN     \
    .section .text.init;      \
    .align 6;                 \
    .weak mtvec_handler;      \
    .globl _start;            \
_start:                       \
    la   t0, _env_trap_vec;   \
    csrw mtvec, t0;           \
    li   TESTNUM, 0;          \
    j    _env_test_start;     \
    .align 2;                 \
_env_trap_vec:                \
    la   t5, mtvec_handler;   \
    beqz t5, 1f;              \
    jr   t5;                  \
1:  RVTEST_FAIL;              \
_env_test_start:

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
