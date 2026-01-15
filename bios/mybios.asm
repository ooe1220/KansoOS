[BITS 16]

; ===============================
; BIOS本体
; ===============================
org 0x0000 ; BIOSは0xF0000-0xFFFFFの64KB(メモリ1MBの一番高い領域)

bios_start:

    cli
    
    ; IVTに登録
    xor ax, ax
    mov es, ax 
    mov word [es:0x13 * 4],     int13h_handler
    mov word [es:0x13 * 4 + 2], cs

    mov ax, 0xF000; セグメントレジスタES及びDS設定
    mov ds, ax
    mov es, ax
    mov ax, 0x0000 ; スタックは0x7C000から下がっていく 30KB程使える
    mov ss, ax
    mov sp, 0x7C00
    
    cld
    
    call vga_init
    
    mov al, 'P'
    ;int 0x10
    call int10_put_char
    
    mov al, 'O'
    call int10_put_char
    
    ;;開発中;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    mov ah, 0x02
    mov al, 1       ; 読み込むセクタ数
    mov ch, 0       ; シリンダ番号
    mov cl, 1       ; セクタ番号（1〜63）
    mov dh, 1       ; ヘッド番号(1枚目表:0、裏:1 2枚目表:3、裏:4)
    mov dl, 0x80    ; 一台目のHDDを読み込む
    mov bx, 0x7C00
    int 0x13

    jmp 0x0000:0x7C00
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cli
hlt_loop:
    hlt
    jmp hlt_loop

disk_error:        
    jmp $

cli
hlt_loop2:
    hlt
    jmp hlt_loop2
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    

%include "vga.asm"
%include "int10h.asm"
%include "int13h.asm"
%include "font_data.asm"
%include "readdisk.asm"

; ===============================
; Reset Vector を FFF0 に置く
; ===============================

times 0xFFF0-($-$$) db 0xFF

; 存在確認hexdump -C mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ===============================
; ROMサイズを64KBに揃える
; ===============================

times 65536-($-$$) db 0xFF

