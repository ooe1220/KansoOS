bits 16
org 0x7c00

start:

    cli
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov ax, 0x7c00
    mov sp, ax
    sti
    
    ; 文字表示の為VRAM(ES : DI)の設定
    mov ax, 0xB800
    mov es, ax
    xor di, di
    
    int 12h
    call print_hex16
    
halt:
    cli
    hlt
    jmp halt
    
; =========================================
; print_hex16
; AXを16進4桁で表示
; =========================================
print_hex16:
    push ax
    push bx
    push cx
    push dx

    mov bx, ax
    mov cx, 4 ; 2バイトの16進数表記は4文字

.loop:
    rol bx, 4

    mov dl, bl
    and dl, 0x0F

    cmp dl, 9
    jbe .num

    add dl, 'A' - 10
    jmp .out

.num:
    add dl, '0'

.out:
    mov al, dl
    call putc

    loop .loop

    pop dx
    pop cx
    pop bx
    pop ax
    ret

; =========================================
; putc
; AL = 文字
; ※diはこの関数内でしか使用していないので保存しない
;   呼ばれる度に増えていく
; =========================================
putc:
    mov ah, 0x0C ; 赤 
    mov [es:di], ax
    add di, 2
    ret


times 510-($-$$) db 0
dw 0xAA55
    
