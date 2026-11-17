bits 16
; A20を有効化する

a20_enable:

    call a20_enable_8042
    call a20_enable_fast
    call a20_check

    ret


a20_enable_8042:
.wait_input_empty:
    in   al, 0x64
    test al, 00000010b   ; 8042 input buffer が空になるまで待つ
    jnz  .wait_input_empty

    mov  al, 0xD0        ; 出力ポート読み出し
    out  0x64, al

.wait_output_full:
    in   al, 0x64
    test al, 00000001b   ; 8042 output buffer に溜まるまで待つ 
    jz   .wait_output_full

    in   al, 0x60        ; 出力ポート読み出し
    or   al, 00000010b   ; A20 (bit1) を立てる
    
    mov  ah, al          ; 値を退避

.wait_input_empty2:
    in   al, 0x64
    test al, 00000010b
    jnz  .wait_input_empty2

    mov  al, 0xD1        ; 出力ポート書き込み
    out  0x64, al
    
    mov  al, ah          ; 値を戻す
    out  0x60, al

    ret
 
 
a20_enable_fast:
    in   al, 0x92
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al
    ret


a20_check:
    push ax
    push si
    push es
    push ds
    push di

    ; 0x0000:0x0500 = 0xAA
    xor ax, ax
    mov es, ax
    mov di, 0x0500
    mov byte [es:di], 0xAA

    ; 0xFFFF:0x0510 = 0x55
    mov ax, 0xFFFF
    mov es, ax
    mov di, 0x0510
    mov byte [es:di], 0x55

    ; 0x0000:0x0500 を読み出し
    xor ax, ax
    mov ds, ax
    mov di, 0x0500
    mov al, [ds:di]

    ; 比較
    cmp al, 0x55
    je .a20_off

    mov si, a20_msg_ok
    call print_string
    
    pop di
    pop ds
    pop es
    pop si
    pop ax
    ret

.a20_off:
    mov si, a20_msg_fail
    call print_string
    
.hang:
    cli
    hlt
    jmp .hang




a20_msg_ok      db "A20 enabled", 0x0D, 0x0A, 0
a20_msg_fail    db "A20 Error", 0x0D, 0x0A, 0
