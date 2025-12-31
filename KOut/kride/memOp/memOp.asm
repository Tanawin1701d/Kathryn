; data
addi r1 r1 1 ; 0x0
addi r2 r2 2 ; 0x4
addi r3 r3 3 ; 0x8
; address
ori r15 r15 128 ; 0xC
; store command <desAddrReg> <Imm> <srcData>
sW r10 4 r1    ; 0x10
sW r10 4 r2    ; 0x14
sW r10 4 r3    ; 0x18
; load command
lw r22 r10 4   ; 0x1C