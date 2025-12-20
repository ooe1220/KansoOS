; ================================
; A20を有効化する
; ================================
; 使用: AXのみ
; ================================

a20_enable_8042:
.wait_input_empty:
    in   al, 0x64
    test al, 00000010b   ; 8042 input buffer が空になるまで待つ
    jnz  .wait_input_empty

    mov  al, 0xD0        ; 出力ポート読み出し
    out  0x64, al

.wait_output_full:
    in   al, 0x64
    test al, 00000001b   ; 8042 output buffer に溜まるまで待つ 
    jz   .wait_output_full

    in   al, 0x60        ; 出力ポート読み出し
    or   al, 00000010b   ; A20 (bit1) を立てる
    
    mov  ah, al          ; 値を退避

.wait_input_empty2:
    in   al, 0x64
    test al, 00000010b
    jnz  .wait_input_empty2

    mov  al, 0xD1        ; 出力ポート書き込み
    out  0x64, al
    
    mov  al, ah          ; 値を戻す
    out  0x60, al

    ret

