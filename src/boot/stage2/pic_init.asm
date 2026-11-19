bits 16

;-----------------------------------------------------------
; pic_init — PIC を ICW で初期化し、全 IRQ をマスクする
;-----------------------------------------------------------
pic_init:
    ; ICW1: エッジトリガ / カスケード / ICW4 あり
    mov al, 0x11
    out 0x20, al            ; マスタ PIC
    out 0xA0, al            ; スレーブ PIC

    ; ICW2: 割り込みベクタの開始番号
    mov al, 0x20
    out 0x21, al            ; マスタ -> IRQ0..7  = 0x20..0x27
    mov al, 0x28
    out 0xA1, al            ; スレーブ-> IRQ8..15 = 0x28..0x2F

    ; ICW3: マスタ/スレーブの配線
    mov al, 0x04            ; マスタの IR2 にスレーブが接続
    out 0x21, al
    mov al, 0x02            ; スレーブ自身の ID = 2
    out 0xA1, al

    ; ICW4: 8086/88 モード
    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; OCW1: 初期状態は全 IRQ マスク
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    mov si, pic_msg
    call print_string

    ret

pic_msg db "PIC initialized", 0x0D, 0x0A, 0