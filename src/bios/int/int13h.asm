[BITS 16]

int13h_handler:

    cmp ah, 0x42 
    ;je atapi_secter_read
    je ata_secter_read 

    ; 未対応
    stc
    mov ah, 0x01
    iret

%include "src/bios/driver/ata.asm"
%include "src/bios/driver/atapi.asm"