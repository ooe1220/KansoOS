; nasm -f bin dump.asm -o dump.bin
; qemu-system-x86_64 -drive format=raw,file=dump.bin
; lsblk
; sudo dd if=dump.bin of=/dev/sdb bs=512 count=10000 conv=notrunc
[bits 16]
org 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    
    ; Miscellaneous Output Register
    mov si, mis_register
    call print_string
    mov dx, 0x3CC
    in  al, dx
    call print_hex_byte
    call print_newline
    call print_newline

    ; Sequencer Registers Dump
    mov si, sec_register
    call print_string
    mov dx, 0x3C4     ; Address Port
    mov cx, 5         ; レジスタ数 (0-4)
    mov bl, 0x00
    call dump_indexed_registers
    call print_newline
    call print_newline

    ; CRT Controller
    mov si, crtc_register
    call print_string
    mov dx, 0x3D4
    mov cx, 25
    mov bl, 0x00
    call dump_indexed_registers
    call print_newline
    call print_newline

    ; Graphics Controller
    mov si, grap_register
    call print_string
    mov dx, 0x3CE
    mov cx, 9
    mov bl, 0x00
    call dump_indexed_registers
    call print_newline
    call print_newline
    
    mov si, attribute_register
    call print_string
    call dump_attribute_registers

    cli
hlt_loop:
    hlt
    jmp hlt_loop
    
   
;--------------------------------
; レジスタダンプ
; DX = ポート番号
; CX = 表示個数
; BL = 開始インデックス(0に設定)
;--------------------------------
dump_indexed_registers:
.next:
    push cx
    push dx

    ; index write
    mov al, bl
    out dx, al
    inc dx            ; data port
    in al, dx

    call print_hex_byte

    ; space
    mov al, ' '
    call print_char

    pop dx
    pop cx

    inc bl
    loop .next
    ret

;--------------------------------
; print AL as hex byte (00-FF)
;--------------------------------
print_hex_byte:
    push ax

    ; high nibble
    shr al, 4
    call print_hex_digit

    pop ax
    and al, 0x0F
    call print_hex_digit
    ret

;--------------------------------
; print single hex digit (0-F)
;--------------------------------
print_hex_digit:
    cmp al, 9
    jbe .num
    add al, 7
.num:
    add al, '0'
    jmp print_char

;--------------------------------
; 1文字出力
;--------------------------------
print_char:
    mov ah, 0x0E
    int 0x10
    ret
    
;--------------------------------
; 文字列出力
;--------------------------------
print_string:
    lodsb          ; DS:SI → AL, SI++
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret
    
;--------------------------------
; 改行 (CR + LF)
;--------------------------------
print_newline:
    push ax

    mov al, 0x0D     ; CR
    call print_char

    mov al, 0x0A     ; LF
    call print_char

    pop ax
    ret
    
;--------------------------------
; Attribute Controller Dump
;--------------------------------
dump_attribute_registers:
    push ax
    push bx
    push cx
    push dx

    mov dx, 0x3DA
    in  al, dx          ; flip-flop reset

    mov dx, 0x3C0
    mov cx, 21
    mov bl, 0x00

.next:
    mov al, bl          ; bit5=0 → 画面を切る
    out dx, al

    mov dx, 0x3C1
    in  al, dx

    call print_hex_byte
    mov al, ' '
    call print_char

    mov dx, 0x3C0
    inc bl
    loop .next

    mov dx, 0x3DA
    in  al, dx          ; flip-flop reset

    mov dx, 0x3C0
    mov al, 0x20        ; bit5=1 → 画面表示復帰
    out dx, al ; この行無いと画面真っ黒

    pop dx
    pop cx
    pop bx
    pop ax
    ret
    
mis_register db "Mis", 0x0D, 0x0A, 0
sec_register db "Sec", 0x0D, 0x0A, 0
crtc_register db "CRTC", 0x0D, 0x0A, 0
grap_register db "Grap", 0x0D, 0x0A, 0
attribute_register db "Attr", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
