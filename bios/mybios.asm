[BITS 16]

; ===============================
; BIOS本体
; ===============================
org 0x0000 ; BIOSは0xF0000-0xFFFFFの64KB(メモリ1MBの一番高い領域)

bios_start:

    cli
    
    ; IVTに登録
    xor ax, ax
    mov es, ax 
    mov word [es:0x10 * 4],     int10h_handler
    mov word [es:0x10 * 4 + 2], cs
    mov word [es:0x13 * 4],     int13h_handler
    mov word [es:0x13 * 4 + 2], cs
    mov word [es:0x19 * 4],     int19h_handler
    mov word [es:0x19 * 4 + 2], cs

    mov ax, 0xF000; セグメントレジスタES及びDS設定
    mov ds, ax
    mov es, ax
    mov ax, 0x0000 ; スタックは0x7C000から下がっていく 30KB程使える
    mov ss, ax
    mov sp, 0x7C00
    
    cld
    
    call vga_init
    
    mov al, 'B'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'I'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'O'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
    mov al, 'S'
    mov ah, 0x1F     ; 青背景・白文字
    int 0x10
    
   ; call kbd_init
        
    sti
    int 0x19
    
cli
hlt_loop:
    hlt
    jmp hlt_loop

%include "bios/vga.asm"
%include "bios/int10h.asm"
%include "bios/int13h.asm"
%include "bios/int19h.asm"
%include "bios/font_data.asm"

; -----------------------------
; 8042 PS/2 キーボード初期化
; -----------------------------
kbd_init:
    cli                     ; 割り込み禁止

    ; 1. 出力バッファをクリア
.clear_outbuf:
    in   al, 0x64           ; ステータス読み取り
    test al, 1              ; bit0 = 出力バッファフル
    jz   .outbuf_cleared
    in   al, 0x60           ; データ読み捨て
    jmp  .clear_outbuf
.outbuf_cleared:

    ; 2. キーボードリセット
.wait_inbuf:
    in   al, 0x64
    test al, 2              ; bit1 = 入力バッファ空き確認
    jnz  .wait_inbuf
    mov  al, 0xFF           ; キーボードリセット
    out  0x60, al

    ; 3. ACK + Self-test OK 読み取り
.wait_ack:
    in   al, 0x64
    test al, 1
    jz   .wait_ack
    in   al, 0x60           ; ACK (0xFA) 読み捨て

.wait_selftest:
    in   al, 0x64
    test al, 1
    jz   .wait_selftest
    in   al, 0x60           ; Self-test OK (0xAA) 読み捨て

    ; 4. LED設定（例: 全て消灯）
.wait_inbuf2:
    in   al, 0x64
    test al, 2
    jnz  .wait_inbuf2
    mov  al, 0xED           ; LEDコマンド
    out  0x60, al

.wait_ack_led:
    in   al, 0x64
    test al, 1
    jz   .wait_ack_led
    in   al, 0x60           ; ACK読み捨て

.wait_inbuf3:
    in   al, 0x64
    test al, 2
    jnz  .wait_inbuf3
    mov  al, 0x00           ; LEDデータ: 全消灯
    out  0x60, al

.wait_ack_led2:
    in   al, 0x64
    test al, 1
    jz   .wait_ack_led2
    in   al, 0x60           ; ACK読み捨て

    ; 5. IRQ1有効化
.wait_inbuf4:
    in   al, 0x64
    test al, 2
    jnz  .wait_inbuf4
    mov  al, 0xAE           ; キーボード割り込み有効化
    out  0x64, al

    sti                     ; 割り込み許可
    ret


; Reset Vector を FFF0 に置く
times 0xFFF0-($-$$) db 0xFF

; 存在確認 hexdump -C ../build/mybios.bin | tail
reset_vector:
    jmp 0xF000:bios_start

; ROMサイズを64KBに揃える
times 65536-($-$$) db 0xFF

