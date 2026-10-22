[BITS 16]

; DS:SIがDAP構造体を指しているためこの2つのレジスタは変更厳禁
; DAP 構造体配置 (DS:SI 基準点):
; +0x00 : db 0x10        ; 大きさ (16バイト)
; +0x01 : db 0x00        ; 予約
; +0x02 : dw sectors     ; 読み込むセクタ数
; +0x04 : dw offset     ; バッファオフセット
; +0x06 : dw segment    ; バッファセグメント
; +0x08 : dd lba_low    ; LBA 下位32bit
; +0x0C : dd lba_high   ; LBA 上位32bit

%define ATA_PRIMARY_DATA_16   0x1F0   ; Data Register (16-bit)
%define ATA_PRIMARY_READ_SECTORS_8    0x1F2   ; Sector Count [7:0]
%define ATA_PRIMARY_LBA_7_0_8     0x1F3   ; LBA [7:0]
%define ATA_PRIMARY_LBA_15_8_8    0x1F4   ; LBA [15:8]
%define ATA_PRIMARY_LBA_23_16_8   0x1F5   ; LBA [23:16]
%define ATA_PRIMARY_LBA_27_24_8   0x1F6   ; Drive / LBA / Master
%define ATA_PRIMARY_STATUS_8      0x1F7   ; Status (R) / Command (W)

ata_secter_read:
    pusha
    push es
    
    ; ATA BSY 待ち
.wait_bsy:
    mov dx, ATA_PRIMARY_STATUS_8
    in al, dx
    test al, 0x80
    jnz .wait_bsy

    ; Drive / LBA / Master
    mov dx, ATA_PRIMARY_LBA_27_24_8
    mov al, 0xE0 ; LBA24-27は0固定
    out dx, al

    ; セクタ数指定（DAP+2 から取得）
    mov dx, ATA_PRIMARY_READ_SECTORS_8
    mov al, [si+2]
    out dx, al
    
    mov ax, [si+8] ; LBA下位32ビット(実際には23BITまでしか使用していない)

    ; LBA 0-7
    mov dx, ATA_PRIMARY_LBA_7_0_8
    out dx, al

    ; LBA 8-15
    mov dx, ATA_PRIMARY_LBA_15_8_8
    mov al, ah              ; LBA[15:8]
    out dx, al

    ; LBA 16-23
    mov dx, ATA_PRIMARY_LBA_23_16_8
    xor al, al              ; LBA[23:16] = 0 ; 0固定
    out dx, al

    ; 読み込み開始の命令をHDDへ送る
    mov dx, ATA_PRIMARY_STATUS_8
    mov al, 0x20
    out dx, al

    ; 読み込むセクタ数
    xor cx, cx
    mov cl, [si+2]

    ; バッファアドレスをDAPから設定（si+4=offset, si+6=segment）
    mov bx, [si+4]      ; buffer offset
    mov ax, [si+6]      ; buffer segment
    mov es, ax

.read_sector:

.wait_drq:
    mov dx, ATA_PRIMARY_STATUS_8
    in al, dx
    test al, 0x08
    jz .wait_drq
    
    mov dx, ATA_PRIMARY_DATA_16
    mov di, 256             ; 512 bytes / 2
.read_word:
    in ax, dx
    mov [es:bx], ax
    add bx, 2
    dec di
    jnz .read_word
    loop .read_sector
    
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
