
int10h_handler:
    pusha
    push es
    
    mov bl, ah              ; BL = 機能番号を退避

    cmp bl, 0x00
    je .mode_set
    
    cmp bl, 0x0E
    je .teletype
    
    jmp .done

.mode_set:
    call vga_mode3_init
    jmp .done

.teletype:

    ;mov dx, 0x3F8
    ;out dx, al

    call tty
    jmp .done    
    
.done:
    pop es
    popa
    iret


