; nasm -f bin mybios.asm -o mybios.bin
; hexdump -C mybios.bin | tail
; qemu-system-i386   -bios mybios.bin   -vga std   -no-reboot   -no-shutdown   -serial stdio

[BITS 16]

; ===============================
; BIOS本体
; ===============================
org 0x0000

bios_start:

    cli
    mov ax, 0xF000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00 
    
    ; --- COM1 init (115200 / 8N1) ---
    mov dx, 0x3F8 + 1      ; IER
    xor al, al
    out dx, al

    mov dx, 0x3F8 + 3      ; LCR
    mov al, 0x80           ; DLAB=1
    out dx, al

    mov dx, 0x3F8 + 0      ; DLL
    mov al, 1              ; 115200 baud
    out dx, al

    mov dx, 0x3F8 + 1      ; DLM
    xor al, al
    out dx, al

    mov dx, 0x3F8 + 3      ; LCR
    mov al, 0x03           ; 8bit, no parity, 1 stop
    out dx, al

    mov dx, 0x3F8 + 2      ; FCR
    mov al, 0xC7
    out dx, al

    mov dx, 0x3F8 + 4      ; MCR
    mov al, 0x0B
    out dx, al

    cld
    
; 1. Miscellaneous Output Register の設定
    mov dx, 0x3C2
    mov al, 0x67
    out dx, al

; 2. Sequencer Registers (Reset)
    mov dx, 0x3C4       ; Sequencer Index ポート

    ; Index 0
    mov al, 0
    out dx, al           ; Index 0 指定
    mov dx, 0x3C5        ; Data ポートに切り替え
    mov al, 0x03
    out dx, al

    ; Index 1
    mov dx, 0x3C4
    mov al, 1
    out dx, al
    mov dx, 0x3C5
    mov al, 0x00
    out dx, al

    ; Index 2
    mov dx, 0x3C4
    mov al, 2
    out dx, al
    mov dx, 0x3C5
    mov al, 0x03
    out dx, al

    ; Index 3
    mov dx, 0x3C4
    mov al, 3
    out dx, al
    mov dx, 0x3C5
    mov al, 0x00
    out dx, al

    ; Index 4
    mov dx, 0x3C4
    mov al, 4
    out dx, al
    mov dx, 0x3C5
    mov al, 0x02
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
    
; --- 2. プレーン2 (フォント領域) への書き込み準備 ---
    mov dx, 0x3C4
    mov ax, 0x0402      ; Sequencer Index 2: Plane 2 に書き込み許可
    out dx, ax
    mov ax, 0x0704      ; Sequencer Index 4: Sequential Access (Chain4解除)
    out dx, ax

    mov dx, 0x3CE
    mov ax, 0x0004      ; Graphics Index 4: Read Plane 2
    out dx, ax
    mov ax, 0x0005      ; Graphics Index 5: Write Mode 0
    out dx, ax
    mov ax, 0x0406      ; Graphics Index 6: Map to 0xA0000 (64KB)
    out dx, ax
    
; --- 3. font.asmの字体をVGAへ登録 ---

    mov dx, 0x3F8
    mov al, '*'
    out dx, al
    
    call register_char
    
    mov dx, 0x3F8
    mov al, '*'
    out dx, al

; --- 4. 通常のテキストモード表示設定に戻す ---
    mov dx, 0x3C4
    mov ax, 0x0302      ; Sequencer Index 2: Plane 0,1 に書き込み許可
    out dx, ax
    mov ax, 0x0304      ; Sequencer Index 4: Odd/Even モード
    out dx, ax

    mov dx, 0x3CE
    mov ax, 0x1005      ; Graphics Index 5: Odd/Even モード
    out dx, ax
    mov ax, 0x0E06      ; Graphics Index 6: Map to 0xB8000
    out dx, ax

    
; --- VRAMにASCIIコードを順番に表示 ---
mov ax, 0xB800
mov es, ax
xor di, di          ; VRAM先頭

mov bl, 0x0E        ; 文字色（黄色）
xor bh, bh          ; BH=0

xor cx, cx          ; CX = 文字コード 0～255

.next_char:
    mov al, cl      ; AL = 文字コード（CLに0～255が入る）
    mov ah, bl      ; AH = 属性
    stosw            ; ES:[DI] = AX
    inc cl           ; 次の文字コード
    cmp cl, 0        ; 256でラップ（CLは8bitなので0に戻る）
    jne .next_char   ; CL != 0 なら続行

cli
hlt_loop:
    hlt
    jmp hlt_loop
    
; QEMU初期化のVGAレジスタダンプ値に基づいたデータ(CRTC 0x0E,0x0Fのカーソル以外)
;seq_data  db 0x03, 0x00, 0x03, 0x00, 0x02
    
crtc_data db 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, \
                 0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00, \
                 0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 0xFF

grap_data db 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0F, 0xFF

attr_data db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
                 0x38, 0x09, 0x3A, 0x0B, 0x3C, 0x0D, 0x3E, 0x0F, \
                 0x0C, 0x01, 0x0F, 0x13, 0x00

register_char:
    mov ax, cs
    mov ds, ax
    mov ax, 0xA000
    mov es, ax

    cld                         ; 方向フラグをクリア（前進）

    mov di, 0x30 * 32           ; 'A' の書き込み開始位置
    mov si, font_0              ; メモリ上のフォントデータ先頭
    mov cx, 75                 ; A(0x41)からN(0x4E)までの14文字分

        mov dx, 0x3F8
    mov al, '*'
    out dx, al

.loop_copy:
    push cx                     ; 外側ループのカウンターを保存
    
    mov dx, 0x3F8
    mov al, '*'
    out dx, al
    
    ; --- 1文字分のフォントデータ(16バイト)をコピー ---
    mov cx, 16
    rep movsb                   ; ds:si から es:di へ16バイト転送
                                ; 実行後、siは次のフォントへ、diは+16進む

    ; --- 次の文字の境界(32バイト目)まで di を調整 ---
    ; 今 di は 16バイト進んだ状態なので、残り16バイト飛ばす
    add di, 16                  
    
    pop cx                      ; カウンターを戻す
    loop .loop_copy             ; 残り文字数分繰り返す
    
    ret

%include "font_data.asm"


; ===============================
; Reset Vector を FFF0 に置く
; ===============================

times 0xFFF0-($-$$) db 0xFF

; 存在確認hexdump -C mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ===============================
; ROMサイズを64KBに揃える
; ===============================

times 65536-($-$$) db 0xFF

