[BITS 16]
[ORG 0x7E00]

start:
    cli                     ; 割り込み禁止

    ; ----------------------------
    ; VGA グラフィックモード設定
    ; mode 13h : 320x200 256色
    ; MODE3に切り替わったことを証明する為一度MODE13に切り替え
    ; ----------------------------
    mov ax, 0x0013
    int 0x10

    ; ----------------------------
    ; VRAM (A000:0000) を赤で塗る
    ; ----------------------------
    mov ax, 0xA000
    mov es, ax
    xor di, di               ; DI = 0

    mov al, 4                ; 色番号 4 = 赤
    mov cx, 320*200          ; 64000 バイト

.fill:
    stosb                    ; ES:DI ← AL, DI++
    loop .fill
    

; 1. Miscellaneous Output Register の設定
    mov dx, 0x3C2
    mov al, 0x67
    out dx, al

; 2. Sequencer Registers (Reset)
    mov dx, 0x3C4
    mov al, 0x00    ; Index 0: Reset
    out dx, al
    inc dx
    mov al, 0x01    ; Binary Reset
    out dx, al

    ; Sequencer Index 1-4 の書き込み
    dec dx
    mov si, seq_data
    mov cx, 4
    mov bl, 1       ; Index 1 から開始
.seq_loop:
    mov al, bl
    out dx, al      ; Index 指定
    inc dx
    lodsb           ; ds:si から値を読み込み
    out dx, al      ; Data 書き込み
    dec dx
    inc bl
    loop .seq_loop

    ; Sequencer Reset 解除
    mov al, 0x00
    out dx, al
    inc dx
    mov al, 0x03    ; Normal Operation
    out dx, al

; 3. CRTC Registers (Unlock Index 0-7)
    ; Index 0x11 の Bit 7 を 0 にして書き込み禁止を解除
    mov dx, 0x3D4
    mov al, 0x11
    out dx, al
    inc dx
    in al, dx
    and al, 0x7F
    out dx, al
    dec dx

    ; CRTC 全データ書き込み (Index 00h - 18h)
    mov si, crtc_data
    mov cx, 25      ; 0x18 までなので 25個
    xor bl, bl      ; Index 0 から
.crtc_loop:
    mov al, bl
    out dx, al
    inc dx
    lodsb
    out dx, al
    dec dx
    inc bl
    loop .crtc_loop
    
; 4. Graphics Controller Registers
    mov dx, 0x3CE
    mov si, grap_data
    mov cx, 9
    xor bl, bl
.grap_loop:
    mov al, bl
    out dx, al
    inc dx
    lodsb
    out dx, al
    dec dx
    inc bl
    loop .grap_loop
    
; 5. Attribute Controller Registers
    ; フリップフロップをリセットするために 0x3DA を Read
    mov dx, 0x3DA
    in al, dx
    
    mov dx, 0x3C0
    mov si, attr_data
    mov cx, 21      ; Palette(16) + Mode Control 等(5)
    xor bl, bl
.attr_loop:
    mov al, bl
    out dx, al      ; Index 書き込み
    lodsb
    out dx, al      ; Data 書き込み
    inc bl
    loop .attr_loop

    ; 映像出力を有効化 (PAS bit をセット)
    mov al, 0x20
    out dx, al

mov ax, 0xB800
mov es, ax
mov word [es:0],  0x0441   ; 'A' 赤
mov word [es:2],  0x0E42   ; 'B' 黄

; --- VRAM全体を 'A' (黄色) で埋める ---
    mov ax, 0xB800      ; テキストモードのVRAMセグメント
    mov es, ax
    xor di, di          ; オフセット 0 から開始
    
    mov ax, 0x0E41      ; AH = 0x0E (黄色属性), AL = 0x41 ('A')
    mov cx, 2000        ; 80文字 × 25行 = 2000文字
    
    cld                 ; 方向に注意（増加方向）
rep stosw               ; ES:[DI] に AX を書き込み、DI を 2 増やす。これを CX 回繰り返す。

.halt:
    hlt
    jmp .halt                ; 完全停止
    
; QEMU初期化のVGAレジスタダンプ値に基づいたデータ(CRTC 0x0E,0x0Fのカーソル以外)
seq_data  db 0x03, 0x00, 0x03, 0x00, 0x02
    
crtc_data db 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, \
                 0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00, \
                 0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 0xFF

grap_data db 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0F, 0xFF

attr_data db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
                 0x38, 0x09, 0x3A, 0x0B, 0x3C, 0x0D, 0x3E, 0x0F, \
                 0x0C, 0x01, 0x0F, 0x13, 0x00
