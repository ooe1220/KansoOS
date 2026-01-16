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

int10h_handler:
    pusha
    push ax

    ; 現在のカーソル取得
    call read_cursor_1d      ; AX = pos

    ; VRAM位置計算
    shl ax, 1                ; pos * 2
    mov di, ax

    mov ax, 0xB800
    mov es, ax

    pop ax
    mov [es:di], al          ; 文字
    mov [es:di+1], ah        ; 属性

    ; カーソルを1進める
    shr di, 1                ; 元のpos
    inc di
    mov ax, di
    call set_cursor_1d
    

    popa
    iret


