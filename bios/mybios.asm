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
    mov word [es:0x10 * 4],     int10h_handler
    mov word [es:0x10 * 4 + 2], cs
    mov word [es:0x13 * 4],     int13h_handler
    mov word [es:0x13 * 4 + 2], cs
    mov word [es:0x19 * 4],     int19h_handler
    mov word [es:0x19 * 4 + 2], cs

    mov ax, 0xF000
    mov ds, ax
    mov es, ax
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x7C00
    
    cld
    
    call vga_mode3_init
    
    ; A20
    in   al, 0x92
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al
    
    mov al, 'B'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'I'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'O'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'S'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
        
    sti
    int 0x19
    
cli
hlt_loop:
    hlt
    jmp hlt_loop

%include "bios/vga.asm"
%include "bios/vbe.asm"
%include "bios/int10h.asm"
%include "bios/int13h.asm"
%include "bios/int19h.asm"
font_8x16: incbin "bios/font_8x16.bin" 

; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

; 存在確認 hexdump -C ../build/mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ROMサイズを64KBに揃える
times 65536-($-$$) db 0xFF

