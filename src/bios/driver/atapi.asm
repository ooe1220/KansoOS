; ============================================================
; ATAPI 拡張読み込みドライバ
;
; DS:SIがDAP構造体を指しているためこの2つのレジスタは変更厳禁
; DAP 構造体配置 (DS:SI 基準点):
; +0x00 : db 0x10        ; 大きさ (16バイト)
; +0x01 : db 0x00        ; 予約
; +0x02 : dw sectors     ; 読み込むセクタ数
; +0x04 : dw offset     ; バッファオフセット
; +0x06 : dw segment    ; バッファセグメント
; +0x08 : dd lba_low    ; LBA 下位32bit
; +0x0C : dd lba_high   ; LBA 上位32bit
;
; ※ ATAPI(CD/DVD)は1セクタ=2048byte。
;    DAPの sectors は「2048byteセクタ数」として扱う（ATA側の512byteと異なる）
;    CDB(READ10)のLBAは2048byte単位 = DAP +0x08 をそのまま使う
; ============================================================

%define ATA_DATA    0x170
%define ATA_FEAT    0x171
%define ATA_BC_LOW  0x174
%define ATA_BC_HIGH 0x175
%define ATA_DEV     0x176
%define ATA_CMD     0x177

%define DEV_MASTER  0xA0
%define ST_ERR      0x01
%define ST_DRQ      0x08
%define ST_BSY      0x80

%define CMD_IDENTIFY    0xA1
%define CMD_PACKET      0xA0
%define CMD_READ_10     0x28

%define SECTOR_SIZE 2048

; ------------------------------------------------------------
; atapi_secter_read
;   DS:SI -> DAP
;   DAPのセクタ数分だけ2048byteずつ ES:BX へ読み込む
; ------------------------------------------------------------
atapi_secter_read:
    pusha
    push es

    ; セクタ数をDAPから（ループカウンタ用にCXへ）
    movzx cx, word [si+2]       ; DAP+2 = セクタ数
    test cx, cx
    jz  .error                  ; 0ならエラー

    mov bx, [si+4]              ; BX = DAP+0x04 (offset)
    mov ax, [si+6]              ; AX = DAP+0x06 (segment)
    mov es, ax                  ; ES ← AX

.next_sector:
    push cx
    push bx                     ; このセクタの宛先 ES:BX を保持

    ; DS:SI(DAP) は変更厳禁。LBAは DAP+0x08 から取り直して渡す
    mov ax, [si+0x08]           ; LBA 下位16bit
    mov dx, [si+0x0A]           ; LBA 上位16bit
    call atapi_read_sector      ; ES:BX へ 1セクタ(2048B) 読む

    pop bx                      ; 宛先を戻して 2048 進める
    add bx, SECTOR_SIZE
    pop cx
    jc  .error                  ; エラーなら終了

    inc dword [si+0x08]         ; DAPのLBA下位を+1
    jnz .no_lba_hi
    inc dword [si+0x0C]         ; 繰り上がり時のみ上位を+1
.no_lba_hi:

    dec cx
    jnz .next_sector

    clc
    xor ah, ah
    jmp .done

.error:
    stc
    mov ah, 0x01
.done:
    pop es
    popa
    iret

; ------------------------------------------------------------
; atapi_read_sector : 1セクタ(2048B)読む
;   入力: DX:AX = LBA (下位32bit)、ES:BX = バッファ
;   返り: CF=0 成功 / CF=1 失敗
;   ※ DS:SI(DAP) は破壊しない
; ------------------------------------------------------------
atapi_read_sector:
    pusha
    push es
    push ds
    push si                     ; ★ DS:SI(DAP) を保護

    push cs
    pop  ds                     ; 作業領域(atapi_cdb)はCS内を参照

    mov [atapi_lba], ax         ; LBA 下位
    mov [atapi_lba+2], dx       ; LBA 上位

    ; --- CDB(READ10) を構築 ---
    mov byte [atapi_cdb+0], CMD_READ_10
    mov byte [atapi_cdb+1], 0
    mov al, [atapi_lba+3]
    mov [atapi_cdb+2], al       ; LBA[31:24]
    mov al, [atapi_lba+2]
    mov [atapi_cdb+3], al       ; LBA[23:16]
    mov al, [atapi_lba+1]
    mov [atapi_cdb+4], al       ; LBA[15:8]
    mov al, [atapi_lba+0]
    mov [atapi_cdb+5], al       ; LBA[7:0]
    mov byte [atapi_cdb+6], 0
    mov byte [atapi_cdb+7], 0
    mov byte [atapi_cdb+8], 1   ; 1セクタずつ読む
    mov byte [atapi_cdb+9], 0
    mov byte [atapi_cdb+10], 0
    mov byte [atapi_cdb+11], 0

    mov di, bx                  ; DI = バッファオフセット（ES は不変）

    ; --- BSY と DRQ が両方下りるまで待つ ---
    mov dx, ATA_CMD
.wait_idle:
    in  al, dx
    test al, (ST_BSY | ST_DRQ)
    jnz .wait_idle

    ; --- Features=0 (PIO) ---
    mov dx, ATA_FEAT
    xor al, al
    out dx, al

    ; --- Byte Count = 2048 (0x0800) ---
    mov dx, ATA_BC_LOW
    xor al, al
    out dx, al
    mov dx, ATA_BC_HIGH
    mov al, 0x08
    out dx, al
    call io_wait

    ; --- Device = Master ---
    mov dx, ATA_DEV
    mov al, DEV_MASTER
    out dx, al
    call io_wait

    ; --- PACKET 発行 ---
    mov dx, ATA_CMD
    mov al, CMD_PACKET
    out dx, al
    call io_wait

    call wait_drq
    jc  .fail

    ; --- CDB(12B=6word) 送出 ---
    mov dx, ATA_DATA
    mov si, atapi_cdb
    mov cx, 6
.cdb_loop:
    lodsw                       ; AX = [DS:SI], SI += 2
    out dx, ax
    loop .cdb_loop

    call wait_drq
    jc  .fail

    mov dx, ATA_CMD
    in  al, dx
    test al, ST_DRQ
    jz  .fail

    ; --- 実際の転送バイト数 ---
    mov dx, ATA_BC_LOW
    in  al, dx
    mov cl, al
    mov dx, ATA_BC_HIGH
    in  al, dx
    mov ch, al                  ; CX = limit
    jcxz .use_default
    cmp cx, SECTOR_SIZE
    jbe .limit_ok
.use_default:
    mov cx, SECTOR_SIZE
.limit_ok:
    and cx, 0xFFFE              ; 偶数へ

    ; --- ES:DI へ転送 ---
    mov dx, ATA_DATA
    shr cx, 1                   ; word数
.recv_loop:
    in  ax, dx
    mov es:[di], ax
    add di, 2
    loop .recv_loop

    clc
    jmp .done
.fail:
    stc
.done:
    pop si
    pop ds
    pop es
    popa
    ret

; ------------------------------------------------------------
; wait_drq : DRQが立つまで待つ（ERR時はCF=1）
; ------------------------------------------------------------
wait_drq:
    mov dx, ATA_CMD
.loop:
    in  al, dx
    test al, ST_ERR
    jnz .fail
    test al, ST_BSY
    jnz .loop
    test al, ST_DRQ
    jz  .loop
    clc
    ret
.fail:
    stc
    ret

io_wait:
    out 0x80, al
    ret

; ------------------------------------------------------------
; 作業領域（DAPではない、ドライバ内部の一時領域）
; ------------------------------------------------------------
    align 2
atapi_lba:  dd 0
atapi_cdb:  times 12 db 0