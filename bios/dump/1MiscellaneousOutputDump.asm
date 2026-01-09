; nasm -f bin 1MiscellaneousOutputDump.asm -o 1MiscellaneousOutputDump.bin
; qemu-system-x86_64 -drive format=raw,file=1MiscellaneousOutputDump.bin

[bits 16]
org 0x7C00

start:
    ; --- 初期設定 ---
    xor ax, ax
    mov ds, ax
    mov es, ax      ; 文字出力用にESも初期化

read_and_print:
    ; --- Miscellaneous Output Register 読み取り ---
    mov dx, 0x3CC
    in  al, dx

    ; --- 取得したALの値を16進数で表示 ---
    push ax         ; 読み取った値を一時保存

    ; 上位4ビットの表示
    shr al, 4
    call print_hex_digit

    ; 下位4ビットの表示
    pop ax
    and al, 0x0F
    call print_hex_digit

    ; 改行
    mov al, 13
    call print_char
    mov al, 10
    call print_char

    cli
hlt_loop:
    hlt
    jmp hlt_loop           ; 停止

; --- 1バイトを16進数1文字に変換して表示 ---
print_hex_digit:
    cmp al, 9
    jbe .is_num
    add al, 7       ; 'A'-'F' への変換（10以上の場合）
.is_num:
    add al, '0'
    call print_char
    ret

; --- 1文字表示（BIOS） ---
print_char:
    mov ah, 0x0E
    int 0x10
    ret

times 510-($-$$) db 0
dw 0xAA55

