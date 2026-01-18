[BITS 16]

; QEMUでは問題ないが本来BIOS領域は書き込み不可の為、0x0500〜へ退避する
; RAM上の変数アドレス定義 (0x0500から順に配置)
%define VAR_BASE 0x0500

sector_count   equ VAR_BASE + 0   ; db (1 byte)  - 読み込むセクタ数
cylinder_num   equ VAR_BASE + 1   ; db (1 byte)  - シリンダ番号
head_num       equ VAR_BASE + 2   ; db (1 byte)  - ヘッド番号
sector_num     equ VAR_BASE + 3   ; db (1 byte)  - セクタ番号

lba_low16      equ VAR_BASE + 4   ; dw (2 byte)  - 変換後のLBA下位16ビット
load_address   equ VAR_BASE + 6   ; dw (2 byte)  - 読み込み先アドレス
lba_calc_tmp   equ VAR_BASE + 8   ; dw (2 byte)  - LBA計算途中結果


int13h_handler:

    ; レジスタ保存
    pusha
    push es
    
    ; ES=0に設定
    push ax
    xor ax, ax
    mov es, ax
    pop ax
    
    mov [es:sector_count], al   ; 読み込むセクタ数    
    mov [es:cylinder_num], ch   ; シリンダ番号
    mov [es:head_num], dh       ; ヘッド番号
    mov [es:sector_num], cl     ; セクタ番号
    mov [es:load_address], bx   ; 読み込み先アドレスオフセット(ES:オフセット)

; ----------------------------
    ; CHS → LBA
    ; LBA = (C * 16 + H) * 63 + (S - 1)

    xor ax, ax
    
    ; --- C * 16 ---
    mov al, [es:cylinder_num] ; AL = cylinder
    mov bl, 16
    mul bl                 ; AX = C * 16
    mov [es:lba_calc_tmp], ax         ; 一時退避

    ; --- + H ---
    mov al, [es:head_num]
    mov ah, 0
    add ax, [es:lba_calc_tmp]         ; AX = C*16 + H
    mov [es:lba_calc_tmp], ax         ; 再度退避

    ; --- * 63 ---
    mov ax, [es:lba_calc_tmp]
    mov bx, 63
    mul bx                 ; DX:AX = (C*16 + H) * 63
    mov [es:lba_calc_tmp], ax         ; 下位16bitだけ退避

    ; --- + (S - 1) ---
    mov al, [es:sector_num]
    dec al                 ; sector - 1
    mov ah, 0
    add ax, [es:lba_calc_tmp]         ; AX = LBA 下位16bit
    ; AXに最終的なLBA下位16bitが入る
    mov [es:lba_low16], ax

; ----------------------------
    ; ATA BSY 待ち
.wait_bsy:
    mov dx, 0x1F7
    in al, dx
    test al, 0x80
    jnz .wait_bsy

    ; Drive / LBA / Master
    mov dx, 0x1F6
    mov al, 0xE0 ; LBA24-27（上位4ビット）は0固定
    out dx, al

    ; セクタ数指定
    mov dx, 0x1F2
    mov al, [es:sector_count]
    out dx, al
    
    mov ax, [es:lba_low16]

    ; LBA 下位 24bit
    mov dx, 0x1F3
    mov al, al              ; LBA[7:0]
    out dx, al

    mov dx, 0x1F4
    mov al, ah              ; LBA[15:8]
    out dx, al

    mov dx, 0x1F5
    xor al, al              ; LBA[23:16] = 0 ; 一旦固定で0にする(今後対応)
    out dx, al

; ----------------------------
    ; 読み込み開始の命令をHDDへ送る
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

; ----------------------------
    ; セクタ数分 読み込み
    xor cx, cx
    mov al, [es:sector_count]
    mov cl, al              ; CX = sector count
    
    ;hlt ;info registers EAX=00000001 ECX=00000001


mov bx, [es:load_address]
.read_sector:
.wait_drq:

    mov dx, 0x1F7
    in al, dx
    test al, 0x08
    jz .wait_drq
    
    mov dx, 0x1F0
    mov di, 256             ; 512 bytes / 2

.read_word:
    in ax, dx
    mov [es:bx], ax
    add bx, 2
    dec di
    jnz .read_word
    
    loop .read_sector
    
    ; ----------------------------
    ; 成功
    clc ; CF=0
    xor ah, ah
    jmp .done

.error:
    stc ; CF=1
    mov ah, 0x01

.done:
    pop es
    popa
    iret

