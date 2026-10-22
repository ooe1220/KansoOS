[bits 16]
org 0x0000 ; BIOSは0xF0000-0xFFFFFの64KB

bios_start:

    cli
    mov ax, 0xF000
    mov ds, ax
    mov es, ax
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x7C00
    cld
    
    ; A20
    in   al, 0x92
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al

 
    ; VBRを0x7C00に読み込む（このコードを上書き）
    push cs
    pop  ds
    mov ah, 0x42
    mov si, mbr_dap ; DS:SIでDAPを指す
    int 0x13
    jc disk_error
    
dap:
    db 0x10            ; DAP サイズ (16バイト)
    db 0x00            ; 予約
    dw 128              ; 読み込みセクタ数 (空HDDに書き込むから64KBまで0埋めは不要)
    dw 0x0000          ; バッファオフセット (ES:BX)
    dw 0x0800          ; バッファセグメント
    dd 126             ; LBA 下位32bit
    dd 0               ; LBA 上位32bit (通常0)
 
    
jmp $

%include "bios/int13h.asm"

; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

reset_vector:
    jmp 0xF000:bios_start

; ROMを64KBに揃える
times 65536-($-$$) db 0xFF

