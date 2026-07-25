/* riscv-tests link map: text at the reset vector; tohost pinned at 0x80200000
   (clear of even the largest test's text/data); data follows tohost. */
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x80000000;
    .text : { *(.text.init) *(.text*) }
    . = 0x80200000;
    .tohost : { *(.tohost) }
    . = ALIGN(64);
    .data : { *(.data*) *(.rodata*) *(.srodata*) *(.sdata*) }
    .bss  : { *(.bss*) *(.sbss*) }
}
