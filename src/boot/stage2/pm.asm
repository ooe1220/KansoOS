bits 16
enter_pm:
    
    ; 返りアドレス保存
    pop ax                      ; near call が積んだ戻り先IP
    mov [pm_return_ip16], ax
    mov [pm_return_cs16], cs

    ; GDT読み込み
    lgdt [gdt_descriptor]

    ; 32bitモード有効化
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32ビットコードへ跳ぶ
    jmp dword 0x08:pm_start

bits 32
pm_start:
    ; データセグメント設定（フラット）
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x900000

    ; 返りアドレス = CS *16 + IP (16bitの時に保存した返りアドレスCS:IPから32ビットのアドレスを計算)
    xor  eax, eax
    mov  ax, [pm_return_cs16]     ; CS
    shl  eax, 4                   ; CS << 4
    movzx ebx, word [pm_return_ip16]
    add  eax, ebx                 ; EAX = CS*16 + IP (20bit 実アドレス)
    push eax
    ret

; ------------------GDT--------------------
gdt_start:
    dq 0x0000000000000000            ; NULL

gdt_code:
    dq 0x00CF9A000000FFFF            ; limit 0-15 / base 0-23 / code / flags

gdt_data:
    dq 0x00CF92000000FFFF            ; limit 0-15 / base 0-23 / data / flags

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ----------RETURN ADDRESS------------

align 4
pm_return_ip16:   dw 0x0000            ; 16bit 戻り先 IP（リアルモード復帰用）
pm_return_cs16:   dw 0x0000            ; 16bit 戻り先 CS

;pm_return_eip32:  dd 0x00000000        ; 32bit 側の戻り先 EIP
