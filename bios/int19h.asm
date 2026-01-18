int19h_handler:

    mov ah, 0x02
    mov al, 1       ; 読み込むセクタ数
    mov ch, 0       ; シリンダ番号
    mov cl, 1       ; セクタ番号（1〜63）
    mov dh, 0       ; ヘッド番号(1枚目表:0、裏:1 2枚目表:3、裏:4)
    mov dl, 0x80    ; 一台目のHDDを読み込む
    mov bx, 0x7C00
    int 0x13  
    
   ; AA55 判定
    xor ax, ax
    mov ds, ax
    mov si, 510
    mov bx, 0x7C00
    mov dx, [bx + si]
    cmp dx, 0xAA55
    jne int19_err

    jmp 0x0000:0x7C00
    
cli
int19_err:
    cli
    hlt
    jmp int19_err
    
    
    
