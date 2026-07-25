/* Bare-metal link map for micro-tests: text at the reset vector, tohost at a
   fixed address the simharness watches (+tohost=0x80001000). */
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x80000000;
    .text : { *(.text.init) *(.text*) }
    . = ALIGN(8);
    .data : { *(.data*) *(.rodata*) *(.srodata*) *(.sdata*) }
    . = 0x80001000;
    .tohost : { *(.tohost) }
    .bss : { *(.bss*) *(.sbss*) }
}
