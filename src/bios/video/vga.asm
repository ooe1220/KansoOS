
; モード13は開発途中
vga_mode13_init:
    cld

    mov dx, 0x3C2          ; MOR
    mov al, 0x63           ; ← Mode13 は 0x63
    out dx, al

    mov dx, 0x3C4          ; Sequencer (4個)
    mov si, mode13_seq_data
    mov cx, 4
    call vga_write_regs


    mov dx, 0x3D4 ; Index 0x11 の Bit 7 を 0 にして書き込み禁止を解除
    mov al, 0x11
    out dx, al
    inc dx
    in al, dx
    and al, 0x7F
    out dx, al
    dec dx
    
    mov dx, 0x3D4          ; CRTC (24個)
    mov si, mode13_crtc_data
    mov cx, 24
    call vga_write_regs

    mov dx, 0x3CE          ; Graphics Controller (9個)
    mov si, mode13_grap_data
    mov cx, 9
    call vga_write_regs

    mov si, mode13_attr_data ; Attribute (20個)
    mov cx, 20
    call vga_attr_write

    call set_palette
    
    mov dx, 0x3C0
    mov al, 0x20
    out dx, al        ; AC のビデオ出力を有効化
    
    ;;;;;;;;;;;;;;;
mov dx, 0x3C4
mov ax, 0x0F02      ; SR02: 全平面書き込み許可（Chain4 下では実質無視されるが慣例）
out dx, ax
mov ax, 0x0E04      ; SR04: Chain4 = 1, Extended Mem = 1
out dx, ax

mov dx, 0x3CE
mov ax, 0x0005      ; GC05: Write Mode 2
out dx, ax
mov ax, 0x0506      ; GC06: Map A0000, Chain4 = 1
out dx, ax
mov ax, 0x0F08      ; GC08: Bit Mask = 全ビット（書き込み時に 0 で塗れなくなるのを防ぐ）
out dx, ax
    ;;;;;;;;;;;;;;;;
    
    ret

vga_mode3_init:
    cld ; 方向フラグ前進
     
    ;======================================================================
    ; Miscellaneous Output Register（MOR）設定
    ;======================================================================
    mov dx, 0x3C2
    mov al, 0x67
    out dx, al

    ;======================================================================
    ; Sequencer設定
    ;======================================================================
    mov dx, 0x3C4
    mov si, mode3_seq_data
    mov cx, 5
    call vga_write_regs
    
    ;======================================================================
    ; CRTC設定
    ;======================================================================
    mov dx, 0x3D4 ; Index 0x11 の Bit 7 を 0 にして書き込み禁止を解除
    mov al, 0x11
    out dx, al
    inc dx
    in al, dx
    and al, 0x7F
    out dx, al
    dec dx
    
    mov si, mode3_crtc_data
    mov cx, 25      ; 0x18 までなので 25個
    call vga_write_regs

    ;======================================================================
    ; Graphics Controller設定
    ;======================================================================
    mov dx, 0x3CE
    mov si, mode3_grap_data
    mov cx, 9
    call vga_write_regs
    
    ;======================================================================
    ; ATTR設定
    ;======================================================================  
    mov si, mode3_attr_data
    call vga_attr_write
    
    ;======================================================================
    ; パレット設定 → [配線を Plane2 に切替] → 字体登録 → [テキスト用に復帰]
    ;======================================================================
    call set_palette
    call vga_plane2_font_open
    call register_char
    call vga_restore_textplane
    
    ;====================================================================== 
    ; 映像出力を有効化
    ;====================================================================== 
    mov dx, 0x3C0 
    mov al, 0x20
    out dx, al
    
    ret

;======================================================================
; パレット設定
;======================================================================

set_palette:
    mov dx, 0x03C8      ; DAC Address Write Mode Register
    mov al, 0           ; インデックス0から開始
    out dx, al

    mov dx, 0x03C9      ; DAC Data Register
    mov si, palette    ; データのアドレスをSIにロード
    mov cx, 64 * 3      ; 64色 * RGB(3バイト) = 192バイト

set_palette_loop:
    lodsb               ; [ds:si]から1バイト読み込んでALへ、SI
    out dx, al          ; ポート0x03C9に書き込み
    loop set_palette_loop
    
    ret
    

;------------------------------------------------------------
; 汎用 VGA レジスタ書き込み
;   DX   = Index ポート (Data は DX+1)
;   DS:SI = データ表
;   CX   = 件数
;   破壊: AL, SI, CX, BL（DX は元の値に戻る）
;------------------------------------------------------------
vga_write_regs:
    push dx             ; 退避
    xor bl, bl          ; 索引
.wr_loop:
    mov al, bl
    out dx, al
    inc dx
    lodsb               ; AL ← [DS:SI] SI++
    out dx, al

    dec dx              ; Index ポートへ戻す
    inc bl
    loop .wr_loop

    pop dx              ; dx を復元
    ret


;======================================================================
; 字体登録
;======================================================================
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
; 字体 RAM へアクセスできるように VRAM 配線を切り替える
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
; 字体登録などでいじった配線を、「テキストモード用」に戻す
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
mode3_seq_data  db 0x03, 0x00, 0x03, 0x00, 0x02
    
mode3_crtc_data db 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, \
                 0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00, \
                 0x9C, 0x8E, 0x8F, 0x28, 0x0F, 0x96, 0xB9, 0xA3, 0xFF

mode3_grap_data db 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0F, 0xFF

mode3_attr_data db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
                 0x38, 0x09, 0x3A, 0x0B, 0x3C, 0x0D, 0x3E, 0x0F, \
                 0x0C, 0x01, 0x0F, 0x13, 0x00
                 
mode13_seq_data  db 0x01, 0x0F, 0x00, 0x0E

mode13_crtc_data db 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, \
                 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3

mode13_grap_data db 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF

mode13_attr_data db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, \
                 0x41, 0x00, 0x0F, 0x00
                               
palette:
    ; 1行あたり8色（24バイト）ずつ配置
    db 0x00,0x00,0x00, 0x00,0x00,0x2a, 0x00,0x2a,0x00, 0x00,0x2a,0x2a, 0x2a,0x00,0x00, 0x2a,0x00,0x2a, 0x2a,0x2a,0x00, 0x2a,0x2a,0x2a
    db 0x00,0x00,0x15, 0x00,0x00,0x3f, 0x00,0x2a,0x15, 0x00,0x2a,0x3f, 0x2a,0x00,0x15, 0x2a,0x00,0x3f, 0x2a,0x2a,0x15, 0x2a,0x2a,0x3f
    db 0x00,0x15,0x00, 0x00,0x15,0x2a, 0x00,0x3f,0x00, 0x00,0x3f,0x2a, 0x2a,0x15,0x00, 0x2a,0x15,0x2a, 0x2a,0x3f,0x00, 0x2a,0x3f,0x2a
    db 0x00,0x15,0x15, 0x00,0x15,0x3f, 0x00,0x3f,0x15, 0x00,0x3f,0x3f, 0x2a,0x15,0x15, 0x2a,0x15,0x3f, 0x2a,0x3f,0x15, 0x2a,0x3f,0x3f
    db 0x15,0x00,0x00, 0x15,0x00,0x2a, 0x15,0x2a,0x00, 0x15,0x2a,0x2a, 0x3f,0x00,0x00, 0x3f,0x00,0x2a, 0x3f,0x2a,0x00, 0x3f,0x2a,0x2a
    db 0x15,0x00,0x15, 0x15,0x00,0x3f, 0x15,0x2a,0x15, 0x15,0x2a,0x3f, 0x3f,0x00,0x15, 0x3f,0x00,0x3f, 0x3f,0x2a,0x15, 0x3f,0x2a,0x3f
    db 0x15,0x15,0x00, 0x15,0x15,0x2a, 0x15,0x3f,0x00, 0x15,0x3f,0x2a, 0x3f,0x15,0x00, 0x3f,0x15,0x2a, 0x3f,0x3f,0x00, 0x3f,0x3f,0x2a
    db 0x15,0x15,0x15, 0x15,0x15,0x3f, 0x15,0x3f,0x15, 0x15,0x3f,0x3f, 0x3f,0x15,0x15, 0x3f,0x15,0x3f, 0x3f,0x3f,0x15, 0x3f,0x3f,0x3f
    
    

