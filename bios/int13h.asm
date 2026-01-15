saved_ax dw 0
saved_dx dw 0

int13h_handler:
    ; --- CHS to LBA変換---
    mov [saved_ax], ax
    mov [saved_dx], dx

    mov ax, 0               ; AX = 0初期化
    mov al, ch              ; AL = シリンダ番号
    mov bl, 63 * 2          ; BL = 1シリンダあたりのセクタ数（63セクタ × 2ヘッド）
    mul bl                  ; AX = AL(シリンダ番号) * BL(126) → AX
    mov di, ax              ; DI = シリンダ部分

    mov dx,[saved_dx]
    mov ax, 0               ; AX = 0初期化  
    mov al, dh              ; AL = ヘッド番号
    mov bl, 63              ; BL = 1トラックあたりのセクタ数
    mul bl                  ; AX = ヘッド × 63
    add di, ax              ; DI = シリンダ部分 + ヘッド部分

    mov al, cl              ; AL = セクタ番号
    dec al                  ; 0ベースに変換
    add di, ax              ; DI = 最終LBA（シリンダ + ヘッド + セクタ）
    ; 結果：DIレジスタにLBAアドレスが格納される
    
    ; LBA 0(第0セクタ)を0x0000:0x7C00へ読み込む
    mov ax, 0x0000
    mov es, ax
    mov di, 0x7C00          ; ES:DI = 0x0000:0x7C00
    
    ; LBA 0 (第0セクタ) の設定
    mov al, 0x01 ;読み込みセクタ数 今回追加したから後から実装
    mov bl, 0x00           ; LBA 0-7
    mov bh, 0x00           ; LBA 8-15
    mov cl, 0x00           ; LBA 16-23
    call read_sector

    jc disk_error

    iret
    
disk_error:        
    jmp $
