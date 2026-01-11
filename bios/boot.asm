[org 0x7C00]
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; --- INT 13h CHS 読み込み ---
    mov ah, 0x02         ; BIOS: Read Sectors
    mov al, 2            ; 読み込むセクタ数 (1)
    mov ch, 0            ; シリンダ = 0
    mov dh, 0            ; ヘッド = 0
    mov cl, 2            ; セクタ = 2 (ブートローダの次)
    mov dl, 0x80         ; ドライブ番号=HDD
    mov bx, 0x7E00       ; ES:BX = 読み込み先
    int 0x13
    jc disk_error
    
    jmp 0x0000:0x7E00    ; 読み込んだコードへジャンプ

disk_error:
    hlt
    jmp disk_error

times 510-($-$$) db 0
dw 0xAA55

