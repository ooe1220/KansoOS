cursor_row dw 0
cursor_col dw 0

int10_put_char:
    pusha
    push ax
    
    ; VRAMアドレス計算
    mov ax, [cursor_row]   ; [ ] が必要
    mov bx, 80
    mul bx
    add ax, [cursor_col]   ; [ ] が必要
    shl ax, 1              ; 1文字2バイト
    mov di, ax

    ; セグメントレジスタの設定（AXを経由する）
    mov ax, 0xB800
    mov es, ax

    ; 文字書き込み
    pop ax
    mov ah, 0x07           ; 属性（白文字・黒背景）
    mov [es:di], al        ; 文字
    mov [es:di+1], ah      ; 属性

    ; カーソル進める
    inc word [cursor_col]  ; word 指定が必要
    cmp word [cursor_col], 80
    jl .done
    mov word [cursor_col], 0
    
    inc word [cursor_row]
    cmp word [cursor_row], 25
    jl .done
    ; 本来はここでスクロール処理などが必要

.done:
    popa
    ;iret
    ret
