bits 16
global start
extern kernel_main

    jmp start
    vbe_info: dd 0 ; modeinfoの先頭アドレス

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    ;sti
    
    mov si, kernei_msg
    call print_string
    
    ;;;;;;;;;;;;;
    cli
    hlt
    ;;;;;;;;;;;;;

    ; A20有効
    call a20_enable_8042
    call a20_enable_fast
    
    ; VBE対応しているか
    mov ax, 0x4F00
    mov di, controllerinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; VBEのVRAMをBIOSに聞く
    mov ax, 0x4F01
    mov cx, 0x114 ; mov cx, 0x115          ; 聞きたいモード
    mov di, modeinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; 800x600
    mov ax, 0x4F02    ; VBE Set Mode
    mov bx, 0x4114 ; mov bx, 0x4115
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ;mov dx, 0x3F8
    ;mov al, 'D'
    ;out dx, al
        
    ; vbe_infoへVBE情報構造体のアドレスを置く
    mov eax, modeinfo
    mov [vbe_info], eax
    
    ; GDT読み込み
    lgdt [gdt_descriptor]

    ; 32bitモード有効化
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32ビットコードへ跳ぶ
    jmp dword 0x08:pm_start
    
no_vbe:
    mov si, error_msg
    call print_string
    ret
    
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret
    
kernei_msg   db "Kernel loaded.", 13, 10, 0
error_msg db "FVBE Error", 0x0D, 0x0A, 0
    
; BIOSが返すVBEの構造体255バイト開始アドレス
controllerinfo: times 0x100 db 0 ; こちらは使わないが受け皿
modeinfo: times 0x100 db 0

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
    
    ;;;;;;;;;;;;;;;;-VBE動作確認-;;;;;;;;;;;;;;;;;;
;    mov edi, 0xE0000000    ; VRAMアドレス
;    mov ecx, 800*600          ; 横幅
;    mov ax, 0x001F         ; RGB565: 青

;.draw_blue_line:
;    mov word [edi], ax     ; 1ピクセル描画
;    add edi, 2             ; 次のピクセルへ
;    loop .draw_blue_line
    
    ; 四隅に赤い点
;    mov edi, 0xE0000000    ; VRAMアドレス
;    mov ax, 0xF800       ; 赤色 (RGB565: 0xF800)  
;    mov word [edi], ax; 1. 左上 (0, 0)
;    mov word [edi + 799 * 2], ax; 2. 右上 (799, 0)
;    mov word [edi + 599 * 800 * 2], ax; 3. 左下 (0, 599)
;    mov word [edi + 599 * 800 * 2 + 799 * 2], ax; 4. 右下 (799, 599)

;    jmp $
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        
    call kernel_main ; kernel.cへ移行

.hlt_loop:
    hlt
    jmp .hlt_loop
; objdump -b binary -m i8086 -D build/kernel.bin | head -60

