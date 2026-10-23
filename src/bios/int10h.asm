
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
    pop es
    popa
    mov ax, 0x004F
    iret

.vbe_01: ; 構造体の内OSで使う値のみ設定
    ; +0x12 x_res
    mov word [es:di + 0x12], 800
    ; +0x14 y_res
    mov word [es:di + 0x14], 600
    ; +0x28 vram_addr
    mov dword [es:di + 0x28], 0xE0000000
    ; +0x32 lfb_pitch
    mov word [es:di + 0x32], 1600
        
    pop es
    popa
    mov ax, 0x004F
    iret  
    
.vbe_02: ; BXに関わらず、OSで使用する800*600*16を固定で設定する
    call vbe_800_600_16_init
    pop es
    popa
    mov ax, 0x004F
    iret 

.done:
    pop es
    popa
    iret


