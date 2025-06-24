; RetStub.asm

.code
RetStub PROC
    xor rcx, rcx
    add rcx, 2
    ret                 
RetStub ENDP
END