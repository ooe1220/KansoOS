register_char:

    ; A = ASCII 0x41
    mov ax, 0xA000
    mov es, ax
    xor di, di

    mov di, 0x41*32
    mov si, font_A
    mov cx, 16
    rep movsb ; [ES:DI] ← [DS:SI]



ret
