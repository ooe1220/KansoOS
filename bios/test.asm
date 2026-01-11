[BITS 16]
[ORG 0x7E00]

start:
    cli                     ; 割り込み禁止
    cld

    ; ----------------------------
    ; VGA グラフィックモード設定
    ; mode 13h : 320x200 256色
    ; BIOSが設定したMODE3の設定及び字体を上書き
    ; その後自力でVGAをMODE3へ再設定
    ; ----------------------------
    mov ax, 0x0013
    int 0x10

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
    ;mov dx, 0x3F8     ; COM1 ポート
    ;mov al, '-'       ; 出す文字
    ;out dx, al
    call register_char



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
                 
;%include "font.asm";
;%include "font_load.asm";

register_char:
    mov ax, 0xA000
    mov es, ax

    mov di, 0x41 * 32           ; 'A' の書き込み開始位置
    mov si, font_A              ; メモリ上のフォントデータ先頭
    mov cx, 14                  ; A(0x41)からN(0x4E)までの14文字分

.loop_copy:
    push cx                     ; 外側ループのカウンターを保存
    
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

font_A:
    db 00011000b
    db 00111100b
    db 00111100b
    db 01100110b
    db 01100110b
    db 01111110b
    db 01111110b
    db 01100110b
    db 01100110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    
font_B:
    db 01111100b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01111100b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01111100b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_C:
    db 00111100b
    db 01100110b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100110b
    db 00111100b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_D:
    db 01111100b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01111100b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_E:
    db 01111110b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01111100b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01111110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_F:
    db 01111110b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01111100b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_G:
    db 00111100b
    db 01100110b
    db 01100000b
    db 01100000b
    db 01100110b
    db 01100110b
    db 01100110b
    db 00111110b
    db 00000110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_H:
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01111110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 01100110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_I:
    db 00111100b
    db 00011000b
    db 00011000b
    db 00011000b
    db 00011000b
    db 00011000b
    db 00011000b
    db 00011000b
    db 00111100b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_J:
    db 00011110b
    db 00001100b
    db 00001100b
    db 00001100b
    db 00001100b
    db 01101100b
    db 01101100b
    db 00111000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_K:
    db 01100110b
    db 01101100b
    db 01111000b
    db 01110000b
    db 01111000b
    db 01101100b
    db 01100110b
    db 01100110b
    db 01100110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_L:
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01100000b
    db 01111110b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_M:
    db 01100011b
    db 01110111b
    db 01111111b
    db 01101011b
    db 01100011b
    db 01100011b
    db 01100011b
    db 01100011b
    db 01100011b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

font_N:
    db 01100011b
    db 01110011b
    db 01111011b
    db 01101111b
    db 01100111b
    db 01100011b
    db 01100011b
    db 01100011b
    db 01100011b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b
    db 00000000b

