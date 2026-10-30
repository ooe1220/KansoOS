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
    
    call is8086
    je not_8086
    jne is_8086
    
is_8086:
    mov si, msg_8086
    call print_string
    jmp halt

not_8086:
    mov si,msg_not_8086
    call print_string
    
halt:
    cli
    hlt
    jmp halt

; =========================================
;   戻り値: ZF=1 → 286以降   ZF=0 → 8086系
; =========================================
is8086:
    push sp          ; 8086系なら「SP-2 した値」が積まれる
    pop  bx
    cmp  bx, sp      ; 一致すれば8086ではない
    ret
    
; =========================================
;   文字列表示
; =========================================    
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

msg_8086     db "8086 detected.", 0x0D, 0x0A, 0
msg_not_8086 db "Not 8086.", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
    
