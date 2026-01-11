register_char:

    mov ax, 0xA000
    mov es, ax
    xor di, di
    
    ; A = ASCII 0x41
    mov di, 0x41*32
    mov si, font_A
    mov cx, 16
    rep movsb

    ; B = ASCII 0x42
    mov di, 0x42*32
    mov si, font_B
    mov cx, 16
    rep movsb

    ; C = ASCII 0x43
    mov di, 0x43*32
    mov si, font_C
    mov cx, 16
    rep movsb
    
    ; D = ASCII 0x44
    mov di, 0x44*32
    mov si, font_D
    mov cx, 16
    rep movsb
    
    ; E = ASCII 0x45
    mov di, 0x45*32
    mov si, font_E
    mov cx, 16
    rep movsb

    ; F = ASCII 0x46
    mov di, 0x46*32
    mov si, font_F
    mov cx, 16
    rep movsb

    ; G = ASCII 0x47
    mov di, 0x47*32
    mov si, font_G
    mov cx, 16
    rep movsb

    ; H = ASCII 0x48
     mov di, 0x48*32
     mov si, font_H
     mov cx, 16
     rep movsb

    ; I = ASCII 0x49
    ; mov di, 0x49*32
    ; mov si, font_I
    ; mov cx, 16
    ; rep movsb

    ; J = ASCII 0x4A
    ; mov di, 0x4A*32
    ; mov si, font_J
    ; mov cx, 16
    ; rep movsb

    ; K = ASCII 0x4B
    ; mov di, 0x4B*32
    ; mov si, font_K
    ; mov cx, 16
    ; rep movsb

    ; L = ASCII 0x4C
    ; mov di, 0x4C*32
    ; mov si, font_L
    ; mov cx, 16
    ; rep movsb

    ; M = ASCII 0x4D
    ; mov di, 0x4D*32
    ; mov si, font_M
    ; mov cx, 16
    ; rep movsb

    ; N = ASCII 0x4E
    ; mov di, 0x4E*32
    ; mov si, font_N
    ; mov cx, 16
    ; rep movsb


ret
