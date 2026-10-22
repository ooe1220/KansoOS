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

    mov dx, 0x3C0
    mov al, 0x20
    out dx, al

    mov dx, 0x01CE
    mov ax, 0x0004
    out dx, ax
    mov dx, 0x01CF
    mov ax, 0x0000
    out dx, ax

    ;800
    mov dx, 0x01CE
    mov ax, 0x0001
    out dx, ax
    mov dx, 0x01CF
    mov ax, 800
    out dx, ax

    ;600
    mov dx, 0x01CE
    mov ax, 0x0002
    out dx, ax
    mov dx, 0x01CF
    mov ax, 600
    out dx, ax

    ;16
    mov dx, 0x01CE
    mov ax, 0x0003
    out dx, ax
    mov dx, 0x01CF
    mov ax, 16
    out dx, ax

    mov dx, 0x01CE
    mov ax, 0x0004
    out dx, ax
    mov dx, 0x01CF
    mov ax, 0x00C1
    out dx, ax
    
    ; GDT読み込み
    lgdt [gdt_descriptor]
    
    ; 32bitモード有効化
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; 32ビットコードへ跳ぶ
    ;jmp 0x08:pm_start
    jmp dword 0x08:(0xF0000 + pm_start)
    
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
    dd 0xF0000 + (gdt_start - $$) 

[bits 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00
    
    mov edi, 0xE0000000    ; VRAMアドレス
    mov ecx, 800*600          ; 横幅
    mov ax, 0x001F         ; RGB565: 青

.draw_blue_line:
    mov word [edi], ax     ; 1ピクセル描画
    add edi, 2             ; 次のピクセルへ
    loop .draw_blue_line
    
jmp $

; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

reset_vector:
    jmp 0xF000:bios_start

; ROMを64KBに揃える
times 65536-($-$$) db 0xFF

