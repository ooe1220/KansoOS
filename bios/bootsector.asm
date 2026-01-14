org 0x7C00

    ; セグメント初期化
    xor ax, ax
    mov ds, ax

; --- VRAM (0xB800:0000) への書き込み準備 ---
mov ax, 0xB800
mov es, ax
;xor di, di          ; 画面左上 (0行目, 0列目) から開始
mov di,0x04
; --- 'H' の表示 ---
mov al, 'H'         ; 文字コード (0x48)
mov ah, 0x0E        ; 属性: 黒背景に黄色 (Yellow on Black)
stosw               ; ES:[DI] に AX(0x0E48) を書き込み、DI を +2 する

; --- 'i' の表示 ---
mov al, 'i'         ; 文字コード (0x69)
mov ah, 0x0E        ; 属性: 黄色
stosw               ; ES:[DI] に AX(0x0E69) を書き込み、DI を +2 する

cli
hlt_loop:
    hlt
    jmp hlt_loop

times 510-($-$$) db 0
dw 0xAA55

