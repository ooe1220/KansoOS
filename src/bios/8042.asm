; ============================================================
; bios_kbd_set1: キーボードを Set1 にして OS に渡せる状態にする
; 破壊: ax, cx, dx
; ============================================================
bios_kbd_set1:
    push    ax
    push    cx
    push    dx

    ; ---- 1. ゴミ捨て ----
.garbage:
    mov     dx, 0x64
    in      al, dx
    test    al, 0x01
    jz      .cfg_read
    mov     dx, 0x60
    in      al, dx          ; 読んで捨てる
    jmp     .garbage

    ; ---- 2. コントローラ設定バイトを読む ----
.cfg_read:
    mov     dx, 0x64
.wait20:
    in      al, dx
    test    al, 0x02
    jnz     .wait20
    mov     al, 0x20
    out     dx, al          ; 0x64 <- 0x20 (読み出し命令)

.wait_data:
    mov     dx, 0x64
    in      al, dx
    test    al, 0x01
    jz      .wait_data
    mov     dx, 0x60
    in      al, dx          ; al = 今の cfg
    mov     ah, al          ; ah に退避

    ; ---- 3. bit4 クリア(キーボード有効) / bit0 セット(IRQ1許可) ----
    and     ah, ~(1 << 4)   ; 1110 1111b
    or      ah,  (1 << 0)   ; 0000 0001b
    ; bit6(変換)は触らない

    ; ---- 書き戻し: 0x64<-0x60, 0x60<-cfg ----
    mov     dx, 0x64
.wait60_1:
    in      al, dx
    test    al, 0x02
    jnz     .wait60_1
    mov     al, 0x60
    out     dx, al          ; 0x64 <- 0x60 (書き込み命令)

    mov     dx, 0x60
.wait60_2:
    mov     dx, 0x64
    in      al, dx
    test    al, 0x02
    jnz     .wait60_2
    mov     dx, 0x60
    mov     al, ah
    out     dx, al          ; 0x60 <- cfg

    ; ---- 4. キーボードに Set 1 を指示: 0xF0 0x01 ----
    mov     dx, 0x64
.waitf0_1:
    in      al, dx
    test    al, 0x02
    jnz     .waitf0_1
    mov     dx, 0x60
    mov     al, 0xF0
    out     dx, al

    mov     dx, 0x64
.wait01_1:
    in      al, dx
    test    al, 0x02
    jnz     .wait01_1
    mov     dx, 0x60
    mov     al, 0x01
    out     dx, al          ; Set 1

    ; ---- ACK(0xFA) を捨てる ----
    mov     dx, 0x64
.wait_ack:
    in      al, dx
    test    al, 0x01
    jz      .wait_ack
    mov     dx, 0x60
    in      al, dx          ; al = 0xFA (捨てる)

    ; ---- 5. 最後のゴミ捨て ----
.final_g:
    mov     dx, 0x64
    in      al, dx
    test    al, 0x01
    jz      .done
    mov     dx, 0x60
    in      al, dx
    jmp     .final_g

.done:
    pop     dx
    pop     cx
    pop     ax
    ret
