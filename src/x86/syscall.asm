.intel_syntax noprefix
.global syscall_handler

syscall_handler:
    pusha

    ## 確認用：VRAMに 'B' を出す
    mov edi, 0xB8002
    mov byte ptr [edi], 'D'
    mov byte ptr [edi+1], 0x0F
    cmp eax, 1              # syscall番号チェック
    jne syscall_done

    push ebx                # 引数（文字列ポインタ）
    call handle_write
    add esp, 4

syscall_done:
    popa
    iret

