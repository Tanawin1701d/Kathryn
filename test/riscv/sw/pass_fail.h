# Shared micro-test macros: write the riscv-tests-style tohost word and spin.
# PASS -> tohost=1 (harness exit 0); FAIL -> tohost=5 (harness exit 2).

#define PASS        \
    li   x28, 1;    \
    la   x29, tohost; \
    sd   x28, 0(x29); \
1:  j    1b

#define FAIL_TO(n)  \
    li   x28, n;    \
    la   x29, tohost; \
    sd   x28, 0(x29); \
1:  j    1b

#define TOHOST_DATA          \
    .section .tohost, "aw";  \
    .globl tohost;           \
    .align 3;                \
tohost: .dword 0

.text
.weak fail
fail:
    FAIL_TO(5)
