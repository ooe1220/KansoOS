[BITS 16]
[ORG 0x7C00]
; dlはMBRからスタック経由で貰う、MBR同様DLをメモリ経由で退避させると失敗する

jmp start

; BPBはmkfs.cで後から埋める
times 59 db 0

start:
    ; CS:IP=0x07C0:0x0000とするBIOS対策、0x0000:0x7C00統一

    jmp 0x0000:real_start
real_start:

    pop dx ; スタック初期化前に取り出す

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, 0x7C00

    mov si, msg_loaded
    call print_string

    ; 8086の場合は起動しない    
    call is8086
    je not_8086
    jne is_8086
is_8086:
    mov si, msg_386only
    call print_string
hang:
    cli
    hlt
    jmp hang
    
not_8086: ; 8086で無かったらOSを起動する
    mov si, msg_cpu_ok
    call print_string
        
load_kernel:
    mov  ah, 0x42      ; LBA拡張リード
    mov  si, dap       ; DAP へのポインタ
    int  0x13
    jc   load_error

    jmp  0x0000:0x8000

; === DAP (Disk Address Packet) ===
dap:
    db 0x10            ; DAP サイズ (16バイト)
    db 0x00            ; 予約
    dw 128              ; 読み込みセクタ数 (空HDDに書き込むから64KBまで0埋めは不要)
    dw 0x0000          ; バッファオフセット (ES:BX)
    dw 0x0800          ; バッファセグメント
    dd 126             ; LBA 下位32bit
    dd 0               ; LBA 上位32bit (通常0)
    
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

; =========================================
;   8086判定 戻り値: ZF=1 → 286以降   ZF=0 → 8086系
; =========================================
is8086:
    push sp          ; 8086系なら「SP-2 した値」が積まれる
    pop  bx
    cmp  bx, sp      ; 一致すれば8086ではない
    ret
    
msg_vbe_fail   db "[VBR] VBE info acquisition failed", 0x0D, 0x0A, 0
msg_loaded db "[VBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
error_msg db "Failed to load KERNEL.BIN", 0x0D, 0x0A, 0

msg_386only   db "KansoOS requires i386 or later. 8086/8088 not supported.", 0x0D, 0x0A, 0
msg_cpu_ok    db "CPU check passed (i386+). Continuing...", 0x0D, 0x0A, 0

; 残りの領域を512バイトまで埋める
times 510-($-$$) db 0
dw 0xAA55

;hexdump -C build/vbr.bin
;objdump -b binary -m i8086 -D build/vbr.bin
