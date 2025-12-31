; this is a comment
addi r1 r1 1    ; 0x0
addi r1 r1 1    ; 0x4
addi r1 r1 1    ; 0x8
addi r2 r2 2    ; 0xc
addi r2 r2 1    ; 0x10
addi r2 r2 1    ; 0x14
addi r3 r3 2    ; 0x18
addi r3 r3 1    ; 0x1c
addi r3 r3 1    ; 0x20
; test predict
beq 4 r2 r3     ; 0x24
addi r3 r1 100  ; 0x28
addi r3 r3 100  ; 0x2c
addi r3 r3 100  ; 0x30