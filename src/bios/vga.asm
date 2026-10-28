
vga_mode3_init:

    cld                         ; 方向フラグ前進
    
; --- パレット設定処理の開始 ---
    mov dx, 0x03C8      ; DAC Address Write Mode Register
    mov al, 0           ; インデックス0から開始
    out dx, al

    mov dx, 0x03C9      ; DAC Data Register
    mov si, palette2    ; データのアドレスをSIにロード
    mov cx, 64 * 3      ; 64色 * RGB(3バイト) = 192バイト

set_palette_loop:
    lodsb               ; [ds:si]から1バイト読み込んでALへ、SI
    out dx, al          ; ポート0x03C9に書き込み
    loop set_palette_loop
    
; 1. Miscellaneous Output Register の設定
    mov dx, 0x3C2
    mov al, 0x67
    out dx, al

    call vga_seq_init
    call vga_crtc_write
    call vga_gc_write
    call vga_attr_write
    
    mov al, 0x20 ; 映像出力を有効化 (PAS bit をセット)
    out dx, al
    
    call vga_plane2_font_open
    call register_char
    call vga_restore_textplane
    ret
    
; ----------以下は関数----------

;======================================================================
; Sequencer(4つのVRAMの使用方法を決める)の登録
;======================================================================
vga_seq_init:

    mov dx, 0x3C4       ; Sequencer Index ポート
    mov si, seq_data
    mov cx, 5
    xor bl, bl
.seq_loop:
    push cx

    mov al, bl
    out dx, al          ; Index 指定
    inc dx              ; 0x3C5 (Data ポート)
    lodsb
    out dx, al

    dec dx              ; 0x3C4 に戻す
    inc bl
    pop cx
    loop .seq_loop
    
    ret

;======================================================================
; CRTC(走査に関する設定)の登録
;======================================================================
vga_crtc_write:
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
    
    ret
    
;======================================================================
; Graphics Controller
;======================================================================
vga_gc_write:
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

    ret
    
    
register_char:
    mov ax, cs
    mov ds, ax
    mov ax, 0xA000
    mov es, ax

    mov di, 0                   ; 'A' の書き込み開始位置
    mov si, font_8x16           ; メモリ上のフォントデータ先頭
    mov cx, 256                   ; ASCII 256文字分

.loop_copy:
    push cx                     ; 外側ループのカウンターを保存
    
    ; --- 1文字分のフォントデータ(16バイト)をコピー ---
    mov cx, 16
    rep movsb                   ; ds:si から es:di へ16バイト転送
                                ; 実行後、siは次のフォントへ、diは+16進む

    ; --- 1文字は32バイト占拠する。8×16字体は16バイトしか使用しない為、残りの16バイトを飛ばす ---
    ; 今 di は 16バイト進んだ状態なので、残り16バイト飛ばす
    add di, 16                  
    
    pop cx                      ; カウンターを戻す
    loop .loop_copy             ; 残り文字数分繰り返す
    
    ret

;======================================================================
; Attribute Controller
;======================================================================
vga_attr_write:
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

    ret


;======================================================================
; フォント RAM (Plane 2) へアクセスできるように VRAM 配線を切り替える
;======================================================================
vga_plane2_font_open:
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
    ret
    
;======================================================================
; フォント登録などでいじった配線を、「テキストモード用」に戻す
;======================================================================
vga_restore_textplane:
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

    ret

; QEMU初期化のVGAレジスタダンプ値に基づいたデータ(CRTC 0x0E,0x0Fのカーソル以外)
seq_data  db 0x03, 0x00, 0x03, 0x00, 0x02
    
crtc_data db 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, \
                 0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00, \
                 0x9C, 0x8E, 0x8F, 0x28, 0x0F, 0x96, 0xB9, 0xA3, 0xFF
                 ;0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 0xFF

grap_data db 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0F, 0xFF

attr_data db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
                 0x38, 0x09, 0x3A, 0x0B, 0x3C, 0x0D, 0x3E, 0x0F, \
                 0x0C, 0x01, 0x0F, 0x13, 0x00
                               
palette2:
    ; 1行あたり8色（24バイト）ずつ配置
    db 0x00,0x00,0x00, 0x00,0x00,0x2a, 0x00,0x2a,0x00, 0x00,0x2a,0x2a, 0x2a,0x00,0x00, 0x2a,0x00,0x2a, 0x2a,0x2a,0x00, 0x2a,0x2a,0x2a
    db 0x00,0x00,0x15, 0x00,0x00,0x3f, 0x00,0x2a,0x15, 0x00,0x2a,0x3f, 0x2a,0x00,0x15, 0x2a,0x00,0x3f, 0x2a,0x2a,0x15, 0x2a,0x2a,0x3f
    db 0x00,0x15,0x00, 0x00,0x15,0x2a, 0x00,0x3f,0x00, 0x00,0x3f,0x2a, 0x2a,0x15,0x00, 0x2a,0x15,0x2a, 0x2a,0x3f,0x00, 0x2a,0x3f,0x2a
    db 0x00,0x15,0x15, 0x00,0x15,0x3f, 0x00,0x3f,0x15, 0x00,0x3f,0x3f, 0x2a,0x15,0x15, 0x2a,0x15,0x3f, 0x2a,0x3f,0x15, 0x2a,0x3f,0x3f
    db 0x15,0x00,0x00, 0x15,0x00,0x2a, 0x15,0x2a,0x00, 0x15,0x2a,0x2a, 0x3f,0x00,0x00, 0x3f,0x00,0x2a, 0x3f,0x2a,0x00, 0x3f,0x2a,0x2a
    db 0x15,0x00,0x15, 0x15,0x00,0x3f, 0x15,0x2a,0x15, 0x15,0x2a,0x3f, 0x3f,0x00,0x15, 0x3f,0x00,0x3f, 0x3f,0x2a,0x15, 0x3f,0x2a,0x3f
    db 0x15,0x15,0x00, 0x15,0x15,0x2a, 0x15,0x3f,0x00, 0x15,0x3f,0x2a, 0x3f,0x15,0x00, 0x3f,0x15,0x2a, 0x3f,0x3f,0x00, 0x3f,0x3f,0x2a
    db 0x15,0x15,0x15, 0x15,0x15,0x3f, 0x15,0x3f,0x15, 0x15,0x3f,0x3f, 0x3f,0x15,0x15, 0x3f,0x15,0x3f, 0x3f,0x3f,0x15, 0x3f,0x3f,0x3f
    
    

