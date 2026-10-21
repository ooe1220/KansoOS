[bits 16]

; ===============================
; BIOS本体
; nasm -f bin mybios.asm -o mybios.bin
; ===============================
org 0x0000 ; BIOSは0xF0000-0xFFFFFの64KB(メモリ1MBの一番高い領域)

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
    
    mov dx, 0x00E9
    mov al, '1'
    out dx, al
    

    ; ----------------------------------------
    ; 2. VGA 基本初期化 (ブランキング解除など)
    ; ----------------------------------------
    ; 属性コントローラのブランキング解除 (0x3C0)
    mov dx, 0x3C0
    mov al, 0x20
    out dx, al

    ; ----------------------------------------
    ; 3. Bochs VBE (DISPI) 初期化
    ; 仕様: Disable -> 設定 -> Enable(LFB)
    ; ----------------------------------------
    ; 3-1. Disable (変更前に必須)
    mov dx, 0x01CE
    mov ax, 0x0004     ; VBE_DISPI_INDEX_ENABLE
    out dx, ax
    mov dx, 0x01CF
    mov ax, 0x0000     ; VBE_DISPI_DISABLED
    out dx, ax

    ; 3-2. XRES = 800
    mov dx, 0x01CE
    mov ax, 0x0001     ; VBE_DISPI_INDEX_XRES
    out dx, ax
    mov dx, 0x01CF
    mov ax, 800
    out dx, ax

    ; 3-3. YRES = 600
    mov dx, 0x01CE
    mov ax, 0x0002     ; VBE_DISPI_INDEX_YRES
    out dx, ax
    mov dx, 0x01CF
    mov ax, 600
    out dx, ax

    ; 3-4. BPP = 16
    mov dx, 0x01CE
    mov ax, 0x0003     ; VBE_DISPI_INDEX_BPP
    out dx, ax
    mov dx, 0x01CF
    mov ax, 16
    out dx, ax

    ; 3-5. Enable (LFB有効 | 通常有効)
    ; 0x01 (ENABLED) + 0x40 (LFB_ENABLED) = 0x41
    mov dx, 0x01CE
    mov ax, 0x0004     ; VBE_DISPI_INDEX_ENABLE
    out dx, ax
    mov dx, 0x01CF
    ;mov ax, 0x0041     ; VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED
    mov ax, 0x00C1
    out dx, ax
    
    ; GDT読み込み
    lgdt [gdt_descriptor]
    
    mov dx, 0x00E9
    mov al, '2'
    out dx, al

    ; 32bitモード有効化
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    mov dx, 0x00E9
    mov al, '3'
    out dx, al
    
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
    ;dd gdt_start
    dd 0xF0000 + (gdt_start - $$) 

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
    
    mov dx, 0x00E9
    mov al, '4'
    out dx, al
    
    ;;;;;;;;;;;;;;;
    
        ; Enableレジスタを読み戻す
    mov dx, 0x01CE
    mov ax, 0x0004
    out dx, ax

    mov dx, 0x01CF
    in  ax, dx

    ; bit 0: VBE enabled
    test ax, 0x0001
    jz .vbe_not_enabled

    ; bit 6: LFB enabled
    test ax, 0x0040
    jz .lfb_not_enabled

    mov dx, 0x00E9
    mov al, 'L'       ; LFB有効
    out dx, al
    jmp .check_done

.vbe_not_enabled:
    mov dx, 0x00E9
    mov al, 'V'       ; VBE自体が無効
    out dx, al
    jmp .check_done

.lfb_not_enabled:
    mov dx, 0x00E9
    mov al, 'N'       ; VBE有効、LFB無効

    out dx, al

.check_done:
    ;;;;;;;;;;;;;;;;
    
    ;mov edi, 0xFD000000    ; QEMU/BochsのLFBアドレス
    mov edi, 0xE0000000    ; QEMU/BochsのLFBアドレス
    mov ecx, 5000           ; 横幅
    mov ax, 0x001F         ; RGB565: 青

.draw_blue_line:
    mov word [edi], ax     ; 1ピクセル描画
    add edi, 2             ; 次のピクセルへ
    loop .draw_blue_line
    
jmp $

; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

; 存在確認 hexdump -C .mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ROMサイズを64KBに揃える
times 65536-($-$$) db 0xFF

