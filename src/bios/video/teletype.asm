read_cursor_1d:
    ; 戻り値 AX = カーソル位置 (0〜1999)

    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al
    mov dx, 0x3D5
    in al, dx
    mov ah, al

    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al
    mov dx, 0x3D5
    in al, dx

    ret

set_cursor_1d:
    ; AX = カーソル位置
    mov cx, ax

    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al
    mov dx, 0x3D5
    mov al, ch
    out dx, al

    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al
    mov dx, 0x3D5
    mov al, cl
    out dx, al

    ret
    
tty:
    mov cl, al              ; 表示文字退避

    cmp cl, 0x0D
    je .cr
    cmp cl, 0x0A
    je .lf

    ; 通常文字
    call read_cursor_1d
    shl ax, 1
    mov di, ax

    mov ax, 0xB800
    mov es, ax

    mov al, cl
    mov byte [es:di], al
    mov byte [es:di+1], 0x07

    shr di, 1
    inc di
    mov ax, di
    call set_cursor_1d
    ret

.cr:
    ; 行頭に戻る（列を0に）
    call read_cursor_1d      ; AX = pos
    mov bx, 80
    xor dx, dx
    div bx                   ; AX = 行番号, DX = 列
    mul bx                   ; AX = 行頭のpos
    call set_cursor_1d
    ret

.lf:
    ; 1行下（列はそのまま）
    call read_cursor_1d      ; AX = pos
    add ax, 80               ; 1行分足すだけ    
    call set_cursor_1d
    ret
    
; div bx → (DX:AX) ÷ BX = AX...DX
; mul bx → AX × BX : DX:AX
; mul bx → AX × BX = DX : AX
    
    
