
int10h_handler:
    pusha
    push es

    cmp ah, 0x00
    je .mode_set
    
    cmp ah, 0x0E
    je .teletype
    
    cmp ax, 0x4F00
    je .vbe_00

    cmp ax, 0x4F01
    je .vbe_01

    cmp ax, 0x4F02
    je .vbe_02
    
    jmp .done

.mode_set:
    call vga_mode3_init
    jmp .done

.teletype:

    ;mov dx, 0x3F8
    ;out dx, al

    call tty
    jmp .done    

.vbe_00: ; 本来は構造体を返すがOS側で使用していないため省略
    mov ax, 0x004F
    jmp .done

.vbe_01:
    nop
    jmp .done   
    
.vbe_02: ; BXに関わらず、OSで使用する800*600*16を固定で設定する
    call vbe_800_600_16_init
    jmp .done   

.done:
    pop es
    popa
    iret


