[BITS 16]
[ORG 0x7C00]

jmp start
nop

times 59 db 0

start:
    ; CS:IP=0x07C0:0x0000とするBIOS対策、0x0000:0x7C00統一

    jmp 0x0000:real_start
real_start:

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, 0x7C00

    push dx
    mov si, msg_loaded
    call print_string
    pop dx    ; ドライブ番号

    mov  ah, 0x42      ; LBA拡張リード
    mov  si, dap_kernel       ; DAP へのポインタ
    int  0x13
    jc   load_error

    mov  ah, 0x42
    mov  si, dap_post
    int  0x13
    jc   load_error

    ; POSTへ飛ぶ
    jmp  0x0000:0x8000

; === DAP (Disk Address Packet) ===
dap_kernel:
    db 0x10            ; DAP サイズ (16バイト)
    db 0x00            ; 予約
    dw 128              ; 読み込みセクタ数 (空HDDに書き込むから64KBまで0埋めは不要)
    dw 0x0000          ; バッファオフセット (ES:BX)
    dw 0x1800          ; バッファセグメント
    dd 72             ; LBA 下位32bit FAT16初期化直後にmcopyでkernel.binを複製するとデータ領域最初のLBA72へ置かれる（今後BPBから計算するように変更予定）
    dd 0               ; LBA 上位32bit (通常0)

dap_post:
    db 0x10
    db 0x00
    dw 128
    dw 0x0000
    dw 0x0800
    dd 72
    dd 0
    
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret
    
; カーネル読み込み失敗
load_error:
    mov si, error_msg
    call print_string

hang_hlt:
    cli
    jmp hang_hlt
    
msg_loaded db "[VBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
error_msg db "Failed to load KERNEL.BIN / POST.BIN", 0x0D, 0x0A, 0

; 何からOSを起動したかを記憶(ファイルシステムの初期マウントに使用)
times 0x1FD-($-$$) db 0   ; 0x1FD (509) まで0埋め
rootdev_sig db 'H' ; HDD

; 残りの領域を512バイトまで埋める
times 510-($-$$) db 0
dw 0xAA55

;hexdump -C build/vbr.bin
;objdump -b binary -m i8086 -D build/vbr.bin
