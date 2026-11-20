bits 16
real_to_prot:
    
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


bits 32

align 4
prot_to_real: ; 32bit mode -> 16bit mode
    ; 16ビットデータセグメントを設定する 0x20 = GDT[4]
    ; D=0なので命令を16ビットととして実行する
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x18:rm_seg

bits 16

rm_seg:

    ; PE = 0
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    
    ; CS = 0として16bitモードへ突入
    jmp 0:real_mode

real_mode:

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7C00

    mov si, enter_rm
    call print_string

    jmp prot_to_real_end

; ------------------GDT--------------------
gdt_start:
    dq 0x0000000000000000            ; NULL

gdt_code:                             ; 0x08
    dq 0x00CF9A000000FFFF           ; limit 0-15 / base 0-23 / code / flags

gdt_data:                              ; 0x10
    dq 0x00CF92000000FFFF            ; limit 0-15 / base 0-23 / data / flags

gdt_code16:                         ; 0x18
    dq 0x00009E000000FFFF            ; 16bit code / base=0 / limit=0xFFFF / G=0 D=0

gdt_data16:                          ; 0x20
    dq 0x000092000000FFFF            ; 16bit data / base=0 / limit=0xFFFF / G=0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ----------RETURN ADDRESS------------

align 4
pm_return_ip16:   dw 0x0000            ; 16bit 戻り先 IP（リアルモード復帰用）
pm_return_cs16:   dw 0x0000            ; 16bit 戻り先 CS

;pm_return_eip32:  dd 0x00000000        ; 32bit 側の戻り先 EIP

enter_rm db "Returned to Real Mode.", 0x0D, 0x0A, 0
