# tcr03 — polled UART TX: wait on LSR.THRE, write "Hello UART\n", then pass.
# run_micro checks the harness stdout against tcr03_hello_uart.expect.
#include "pass_fail.h"

#define UART_BASE 0x10000000

.section .text.init
.globl _start
_start:
    la   a0, msg
next_char:
    lbu  a1, 0(a0)
    beqz a1, done
    li   t0, UART_BASE
wait_thre:
    lbu  t1, 5(t0)                # LSR
    andi t1, t1, 0x20             # THRE
    beqz t1, wait_thre
    sb   a1, 0(t0)                # THR
    addi a0, a0, 1
    j    next_char
done:
    PASS

.data
msg: .asciz "Hello UART\n"

TOHOST_DATA
