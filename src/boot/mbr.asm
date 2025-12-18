[BITS 16]
[ORG 0x7C00]

start:
    ; 1. MBRを安全な領域（0x0600）へ退避
    xor ax, ax
    mov ds, ax
    mov si, 0x7C00
    mov di, 0x0600
    mov cx, 512
    rep movsb

    ; 2. セグメントレジスタの初期化
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov si, msg_loading
    call print_string
    
    ; 0x0600にジャンプして処理を続行
    jmp 0x0000:(continue - start + 0x0600)

continue:

    mov si, msg_relocated
    call print_string

    ; 4. VBRを0x7C00に読み込む（このコードを上書き）
    mov ah, 0x02
    mov al, 1       ; 読み込むセクタ数
    mov ch, 0       ; シリンダ番号
    mov cl, 1       ; セクタ番号（1〜63）
    mov dh, 1       ; ヘッド番号
    mov dl, 0x80    ; 一台目のHDDを読み込む
    mov bx, 0x7C00
    int 0x13
    jc disk_error

    ; 5. 0x7C00に読み込んだVBRへ跳ぶ
    jmp 0x0000:0x7C00

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
    mov si, msg_disk_error
    call print_string
    jmp $

msg_loading db "[MBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
msg_relocated db "[MBR] Relocated to 0x0000:0x0600", 0x0D, 0x0A, 0
msg_disk_error db "Disk read error!", 0x0D, 0x0A, 0

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

