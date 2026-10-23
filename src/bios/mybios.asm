[BITS 16]

; ===============================
; BIOS本体
; ===============================
org 0x0000 ; BIOSは0xF0000-0xFFFFFの64KB(メモリ1MBの一番高い領域)

bios_start:

    call cpupost

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
    
    ; テキストmode3
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; A20
    in   al, 0x92
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al
   
    mov si, msg_bios
    call print_string
        
    sti
    int 0x19
    
cli
hlt_loop:
    hlt
    jmp hlt_loop

%include "src/bios/cpupost.asm"
%include "src/bios/vga.asm"
%include "src/bios/vbe.asm"
%include "src/bios/int10h.asm"
%include "src/bios/teletype.asm"
%include "src/bios/int13h.asm"
%include "src/bios/int19h.asm"
font_8x16: incbin "src/bios/font_8x16.bin" 

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret
    
msg_bios   db "Custom BIOS booting", 0x0D, 0x0A, 0

; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

; 存在確認 hexdump -C ../build/mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ROMサイズを64KBに揃える
times 65536-($-$$) db 0xFF

