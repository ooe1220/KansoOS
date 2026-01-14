[BITS 16]

; ===============================
; BIOS本体
; ===============================
org 0x0000

bios_start:

    cli
    mov ax, 0xF000
    mov ds, ax
    mov es, ax
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x7C00 
    
    cld
    
    call vga_init
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; LBA 2(第2セクタ)を0x0000:0x7E00へ読み込む
    mov ax, 0x0000
    mov es, ax
    mov di, 0x7C00          ; ES:DI = 0x0000:0xCE00
    
    ; LBA 1 (第2セクタ) の設定
    mov bl, 0x00           ; LBA 0-7
    mov bh, 0x00           ; LBA 8-15
    mov cl, 0x00           ; LBA 16-23
    call read_sector

    
    jc disk_error
    
    mov dx, 0x3F8
    mov al, 'l'
    out dx, al
    
    jmp 0x0000:0x7C00
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cli
hlt_loop:
    hlt
    jmp hlt_loop
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
disk_error:
    
    mov dx, 0x3F8
    mov al, 'E'
    out dx, al    
    
    jmp $
    
cli
hlt_loop2:
    hlt
    jmp hlt_loop2
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    

%include "vga.asm"
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

