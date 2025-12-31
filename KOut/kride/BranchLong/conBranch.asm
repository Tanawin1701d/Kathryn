; this is a comment
addi r2 r2 1       ; 0x00
addi r3 r3 1       ; 0x04
; test predict
beq 4 r1 r1        ; 0x08
beq 4 r1 r1        ; 0x0c
beq 4 r1 r1        ; 0x10
beq 4 r1 r1        ; 0x14
beq 4 r1 r1        ; 0x18
beq 4 r1 r1        ; 0x1c
beq 4 r1 r1        ; 0x20
addi r4 r0 100     ; 0x24