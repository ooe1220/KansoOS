; nasm -f bin 4GraphicsControllerDump.asm -o 4GraphicsControllerDump.bin
; qemu-system-x86_64 -drive format=raw,file=4GraphicsControllerDump.bin

[bits 16]
org 0x7C00

start:
    ; --- 初期設定 ---
    xor ax, ax
    mov ds, ax
    mov es, ax      ; 文字出力用にESも初期化

    ; 読み取りインデックスの初期化
    mov bl, 0x00    ; インデックス 0 から開始

read_and_print:
    ; --- VGAレジスタ読み取り ---
    mov dx, 0x3CE   ; Address Port
    mov al, bl      ; 現在のインデックス
    out dx, al
    inc dx          ; Data Port (0x3CF)
    in al, dx       ; 値をALに取得
    
    ; --- 取得したALの値を16進数で表示 ---
    push ax         ; 読み取った値を一時保存
    
    ; 上位4ビットの表示
    shr al, 4       ; 右に4シフトして上位4ビットを取り出す
    call print_hex_digit
    
    ; 下位4ビットの表示
    pop ax          ; 値を戻す
    push ax         ; 次のために再度保存
    and al, 0x0F    ; 下位4ビットを取り出す
    call print_hex_digit
    
    ; 空白を表示
    mov al, ' '
    call print_char

    ; --- ループ制御 ---
    pop ax          ; スタックを戻す
    inc bl          ; 次のインデックス
    cmp bl, 9       ; 9個分終わったか？
    jne read_and_print

    cli
hlt_loop:
    hlt
    jmp hlt_loop           ; 停止

; --- 1ビットを16進数文字に変換して表示する関数 ---
print_hex_digit:
    cmp al, 9
    jbe .is_num
    add al, 7       ; 'A'-'F' への変換（10以上の場合）
.is_num:
    add al, '0'     ; '0'-'9' への変換

print_char:
    mov ah, 0x0E    ; BIOS 1文字出力機能
    int 0x10        ; 画面出力
    ret

times 510-($-$$) db 0
dw 0xAA55
