; VGA Graphics Controller Mode 3 設定
; インデックスレジスタ: 0x3CE, データレジスタ: 0x3CF

; --- 0x00: Set/Reset ---
; bit 0-3 : 各プレーンのセット/リセット（0=リセット）
; bit 4-7 : 未使用
mov dx, 0x3CE
mov al, 0x00       ; Set/Reset レジスタ選択
out dx, al
mov dx, 0x3CF
mov al, 0x00       ; すべてリセット
out dx, al


; --- 0x01: Enable Set/Reset ---
; bit 0-3 : 各プレーンに対して Set/Reset を有効にする（1=有効）
; bit 4-7 : 未使用
mov dx, 0x3CE
mov al, 0x01       ; Enable Set/Reset レジスタ選択
out dx, al
mov dx, 0x3CF
mov al, 0x00
out dx, al

; --- 0x02: Color Compare ---
; bit 0-3 : 比較するカラー（Mode3では未使用）
; bit 4-7 : 未使用
mov dx, 0x3CE
mov al, 0x02       ; Color Compare レジスタ選択
out dx, al
mov dx, 0x3CF
mov al, 0x00
out dx, al

; --- 0x03: Data Rotate ---
; bit 0-2 : 回転量（0=回転なし）
; bit 3   : 論理演算（0=通常書き込み）
mov dx, 0x3CE
mov al, 0x03       ; Data Rotate レジスタ選択
out dx, al
mov dx, 0x3CF
mov al, 0x00
out dx, al

; --- 0x04: Read Map Select ---
; bit 0-1 : 読み込みプレーン選択（Mode3では通常0）
; bit 2-7 : 未使用
mov dx, 0x3CE
mov al, 0x04       ; Read Map Select
out dx, al
mov dx, 0x3CF
mov al, 0x00       ; プレーン0
out dx, al

; --- 0x05: Graphics Mode ---
; 不明　BIOS初期化後ダンプのまま
mov dx, 0x3CE
mov al, 0x05       ; Graphics Mode
out dx, al
mov dx, 0x3CF
;mov al, 0x10
mov al, 0x00
out dx, al

; --- 0x06: Miscellaneous Graphics ---
; bit 0-1 : 色を決める（通常0）
; bit 2   : 0=プレーン単位, 1=その他
; bit 3-7 : 未使用
; BIOS初期化後ダンプでは未使用のところに1が立っていた
mov dx, 0x3CE
mov al, 0x06       ; Misc Graphics
out dx, al
mov dx, 0x3CF
;mov al, 0x0E
mov al, 0x0D
out dx, al

; --- 0x07: Color Don't Care ---
; bit0-3 : 無視する色（Mode3では通常0）
; bit4-7 : 未使用
mov dx, 0x3CE
mov al, 0x07       ; Color Don't Care
out dx, al
mov dx, 0x3CF
mov al, 0x0F       ; すべての色を関心対象
out dx, al

    mov dx, 0x3F8
    mov al, '*'
    out dx, al

; --- 0x08: Bit Mask ---
; bit0-7 : 書き込みマスク（1=書き込み許可）
mov dx, 0x3CE
mov al, 0x08       ; Bit Mask
out dx, al

    mov dx, 0x3F8
    mov al, '*'
    out dx, al

mov dx, 0x3CF
mov al, 0xFF       ; すべて書き込み許可
out dx, al
