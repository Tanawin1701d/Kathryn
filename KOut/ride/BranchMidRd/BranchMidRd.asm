addi r1 r0 1       ; 0x00
addi r2 r0 1       ; 0x04
; we intend to test the renaming table due to the jumping instruction with having destination register
jal r20 4          ; 0x08
jal r25 4          ; 0x0C
addi r3 r0 1       ; 0x10
addi r4 r0 1       ; 0x14