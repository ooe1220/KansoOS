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

    mov si, dap
    mov ah, 0x42
    int 0x13
    jc disk_fail

    ; ここに来れば 0x8000 に 64KB 読み込み完了
    jmp 0x0000:0x8000

; -------- DAP --------
dap:
    db 0x10          ; 大きさ
    db 0             ; 予約
    dw 32            ; 転送ブロック数
    dw 0x8000        ; offset
    dw 0x0000        ; segment（物理 0x8000）
    dq 35            ; KERNEL.BIN の LBA = 35
    
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
error_msg db "Failed to load KERNEL.BIN", 0x0D, 0x0A, 0

times 2048-($-$$) db 0
