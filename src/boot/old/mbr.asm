;QEMUの不可解な動作
; dlをメモリ経由で退避させて復元してからint13すると失敗する→一旦int13実行までdlには触らない
;(xor ax,ax xor ax,ax)及び(sub ax,ax xor ax,ax)は失敗するが、(mov ax,0 xor ax,ax)には成功する　理由不明
[BITS 16]
[ORG 0x7C00]

start
jmp code

boot_drive:
dw 0

code:
    ; CS:IP=0x07C0:0x0000とするBIOS対策、0x0000:0x7C00統一
    jmp 0x0000:real_start
real_start:

    ;セグメントレジスタの初期化
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; MBRを安全な領域（0x0600）へ退避
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, 0x7C00
    mov di, 0x0600
    mov cx, 512
    rep movsb

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

    mov si, msg_not_8086
    call print_string
    
    mov si, msg_loading - start + 0x0600
    call print_string
        
    ; 途中で0x600へ以降してはいるが、callやjmp nearは相対アドレスなので他のラベルはそのまま使える
    ; 0x0600へ移行。call/jmpは相対なので分岐はそのまま動くが、jmp farは再計算が必要
    jmp 0x0000:(continue - start + 0x0600)

continue:

    mov si, msg_relocated - start + 0x0600
    call print_string
    
    ; VBRを0x7C00に読み込む（このコードを上書き）
    mov ah, 0x42
    mov si, mbr_dap
    int 0x13
    jc disk_error

    push dx           ; MBRへドライブ番号を渡す
    jmp 0x0000:0x7C00 ; VBRへ移行

; === DAP ===
mbr_dap:
    db 0x10
    db 0x00
    dw 1                  ; 1セクタ（VBR = 512バイト）
    dw 0x7C00             ; バッファオフセット
    dw 0x0000             ; バッファセグメント
    dd 63                 ; LBA = パーティションテーブルと同じ値
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

disk_error:
    mov si, msg_disk_error - start + 0x0600
    call print_string
    jmp $

; =========================================
;   8086判定 戻り値: ZF=1 → 286以降   ZF=0 → 8086系
; =========================================
is8086:
    push sp          ; 8086系なら「SP-2 した値」が積まれる
    pop  bx
    cmp  bx, sp      ; 一致すれば8086ではない
    ret

msg_loading db "[MBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
msg_relocated db "[MBR] Relocated to 0x0000:0x0600", 0x0D, 0x0A, 0
msg_disk_error db "Disk read error!", 0x0D, 0x0A, 0

msg_386only   db "[MBR] This OS is 386-only. Booting from an 8086 is not supported.", 0x0D, 0x0A, 0
msg_not_8086  db "[MBR] CPU check passed. Continuing...", 0x0D, 0x0A, 0

; MBRの残りを埋める
times 446-($-$$) db 0

; --- 1つ目のパーティション ---
db 0x80               ; アクティブパーティション
db 0x01, 0x01, 0x00   ; CHS start = C=0, H=1, S=1
db 0x06               ; パーティションタイプ（FAT16）
db 0xFF, 0xFF, 0xFF   ; CHS終了位置（最大値で無視）
dd 63                ; 開始 LBA
dd 1985              ; パーティションの大きさ 2048 - 63 = 1985(セクタ)

; --- 残りの3つの空パーティション ---
times 16*3 db 0       ; 3個分 ×16バイトを0で埋める

dw 0xAA55
; nasm -f bin src/boot/mbr.asm -o build/mbr.bin -l build/mbr.lst
; cat build/mbr.lst
; hexdump -C build/mbr.bin
