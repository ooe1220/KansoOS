[BITS 16]
[ORG 0x7C00]

; jmp start及びBPBはmkfs.cで後から埋める
times 62 db 0

start:

    ; CS:IP=0x07C0:0x0000とするBIOS対策、0x0000:0x7C00統一
    ; MBRでもやっているが将来パーティションを無くしても残る様に
    jmp 0x0000:real_start
real_start:

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, 0x7C00

    mov si, msg_loaded
    call print_string
    
    ; VBEのVRAMをBIOSに聞く
    mov ax, 0x4F01
    mov cx, 0x115          ; 聞きたいモード
    mov di, modeinfo
    int 0x10
    
    ; 800x600 24bpp (モード 0115h) 
    mov ax, 0x4F02    ; VBE Set Mode
    mov bx, 0x4115    ; 0115h (24bpp) + ビット14
    int 0x10
    
    ; vbe_vram_infoへVBE情報構造体のアドレスを置く
    mov eax, modeinfo
    mov [vbe_vram_info], eax
    
    ; 表示の為にVGA MODE3へ戻す
    mov ax, 0x0003
    int 0x10
    
    ; 20261009 CHS -> LBA方式へ変更
    ; kernelは64セクタ分(32KB)，LBA=126
    ; KERNEL.BIN は LBA 126 セクタ目から始まる
    ;mov ah, 0x02
    ;mov al, 63 ; 読み込みセクタ数 1トラック分（32KB）
    ;mov ch, 0 ; シリンダ
    ;mov cl, 1 ; セクタ
    ;mov dh, 2 ; ヘッド
    ;mov dl, 0x80 ; HDD
    ;mov bx, 0x8000 ; メモリ0x8000番地へ読み込む
    ;int 0x13
    ;jc load_error
    ;jmp 0x0000:0x8000 ; kernelの開始アドレスへ跳ぶ
    
    
load_kernel:
    push 0x8000        ; バッファセグメント
    pop  es
    mov  bx, 0        ; ES:BX = 0x8000:0x0000

    mov  ah, 0x42      ; LBA拡張リード
    mov  dl, 0x80
    mov  si, dap       ; DAP へのポインタ
    int  0x13
    jc   load_error

    jmp  0x0000:0x8000

; === DAP (Disk Address Packet) ===
dap:
    db 0x10            ; DAP サイズ (16バイト)
    db 0x00            ; 予約
    dw 64              ; 読み込みセクタ数
    dw 0x8000          ; バッファオフセット (ES:BX)
    dw 0x0000          ; バッファセグメント
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
    
; VBE情報取得失敗
vbe_fail:
    mov si, msg_vbe_fail
    call print_string
    jmp hang_hlt

; カーネル読み込み失敗
load_error:
    mov si, error_msg
    call print_string

hang_hlt:
    cli
    jmp hang_hlt
    
msg_vbe_fail   db "[VBR] VBE info acquisition failed", 0x0D, 0x0A, 0
msg_loaded db "[VBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
error_msg db "Failed to load KERNEL.BIN", 0x0D, 0x0A, 0

; VBE返り値44バイト modeinfo + 0x28 = VRAM開始アドレス
modeinfo: resb 0x2C

times 506-($-$$) db 0
vbe_vram_info: resd 1 ; VBE VRAM開始アドレス

; 残りの領域を512バイトまで埋める
;times 510-($-$$) db 0

dw 0xBB66 ; メモリ上でMBRとVBRを区別する為、便宜的に(本来はこんな書き方をすべきではない)

