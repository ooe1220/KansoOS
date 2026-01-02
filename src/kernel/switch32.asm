bits 16
global start
extern kernel_main


start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; VGAテキストモード設定(mode3)
    mov ax, 0x0003
    int 0x10 
    
    ; A20有効
    call a20_enable_8042

    ; GDT読み込み
    lgdt [gdt_descriptor]

    ; 32bitモード有効化
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32ビットコードへ跳ぶ
    jmp 0x08:pm_start

; -----------------------------
gdt_start:
    dq 0x0000000000000000            ; NULL

gdt_code:
    dw 0xFFFF                        ; limit 0-15
    dw 0x0000                        ; base 0-15
    db 0x00                          ; base 16-23
    db 0x9A                          ; code segment
    db 0xCF                          ; flags
    db 0x00                          ; base 24-31

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92                          ; data segment
    db 0xCF
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    
%include "src/kernel/a20.asm"; A20有効処理の実装

; -----------------------------
[bits 32]
pm_start:
    ; データセグメント設定（フラット）
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00
    
    call kernel_main ; kernel.cへ移行

.hlt_loop:
    hlt
    jmp .hlt_loop

