BITS 16
ORG 0x7C00

start:

    jmp 0x0000:real_start
real_start:

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    push dx
    
    mov si, msg_boot
    call print_string
    
    pop dx

    ; xp /64x 0x8000
    mov si, dap_post
    mov ah, 0x42
    int 0x13
    jc disk_fail

     ; xp /64x 0x18000
    mov si, dap_kernel
    mov ah, 0x42
    int 0x13
    jc disk_fail

    ; POSTへ飛ぶ
    jmp 0x0000:0x8000

; -------- DAP --------
dap_kernel:
    db 0x10          ; 大きさ
    db 0             ; 予約
    dw 32            ; 転送ブロック数 64KB分
    dw 0x0000        ; offset
    dw 0x1800        ; segment（0x1800×16+0 = 物理 0x18000）
    dq 38;            ; KERNEL.BIN の LBA 今後PVD→ルートを辿って探す様に変更

dap_post:
    db 0x10
    db 0
    dw 32
    dw 0x0000
    dw 0x0800
    dq 71;
    
disk_fail:
    mov si, error_msg
    call print_string

hang:
    cli
    hlt
    jmp hang

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

msg_boot   db "CD Boot Loader Started", 13, 10, 0
error_msg db "Failed to load KERNEL.BIN / POST", 0x0D, 0x0A, 0

test_msg db "TEST", 0x0D, 0x0A, 0

; 何からOSを起動したかを記憶(ファイルシステムの初期マウントに使用)
; 1 セクタに収まらなくなったら ↓ を有効化
; jmp rootdev_sig_skip
times 0x1FD-($-$$) db 0   ; 0x1FD (509) まで0埋め
rootdev_sig db 'C' ; CD/DVD

times 2048-($-$$) db 0
