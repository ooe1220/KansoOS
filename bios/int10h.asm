cursor_row dw 0
cursor_col dw 0

; VGAカーソル位置を読み込む
read_vga_cursor:
    pusha

    ; カーソル位置の上位バイト
    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al
    mov dx, 0x3D5
    in al, dx
    mov ah, al
    
    ; カーソル位置の下位バイト
    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al
    mov dx, 0x3D5
    in al, dx
    
    ;hlt ; info registers
    
    ; 1次元位置を2次元座標に変換
    mov bx,80
    xor dx,dx
    div bx
    
    ;mov bl, 80           ; BLレジスタに画面幅80を設定
    ;div bl               ; AX ÷ BL → 商がAL（行）、余りがAH（列）
    
    mov [cursor_row], ax;al
    mov [cursor_col], dx;ah
    
    popa
    ret
    
; VGAカーソル位置を設定する
set_vga_cursor:
    pusha
    
    ; 2次元座標を1次元位置に変換
    mov ax, [cursor_row]
    mov bx, 80               ; 画面幅80をBXに設定
    mul bx                   ; AX = cursor_row × 80
    add ax, [cursor_col]     ; AX = (cursor_row × 80) + cursor_col
    
    mov cx, ax ; AXを退避
    
    ; 上位8ビットを設定
    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al
    mov dx, 0x3D5
    mov al, ch              ; 位置の上位バイト
    out dx, al
    
    ; 下位8ビットを設定
    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al
    mov dx, 0x3D5
    mov al, cl              ; 位置の下位バイト
    out dx, al
    
    popa
    ret

int10_put_char:
    pusha
    push ax
    
    ; VGAから現在のカーソル位置を読み込む
    call read_vga_cursor
        
    ; VRAMアドレス計算
    mov ax, [cursor_row]
    mov bx, 80
    mul bx
    add ax, [cursor_col]
    shl ax, 1              ; 1文字2バイト
    mov di, ax

    ; セグメントレジスタの設定（AXを経由する）
    mov ax, 0xB800
    mov es, ax

    ; 文字書き込み
    pop ax
    mov [es:di], al        ; 文字
    mov [es:di+1], ah      ; 属性

    ; カーソル進める
    inc word [cursor_col] 
    cmp word [cursor_col], 80
    jl .done
    mov word [cursor_col], 0
    
    inc word [cursor_row]
    cmp word [cursor_row], 25
    jl .done
    ; 本来はここでスクロール処理などが必要


.done:
    call set_vga_cursor
    popa
    ;iret
    ret
