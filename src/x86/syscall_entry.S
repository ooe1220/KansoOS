# 初めはC言語中にインラインアセンブラとして実装していたが、
# C言語をコンパイルする際にPIC(アドレス間接参照)の影響により
# コンパイラが勝手に`add $0x1,%eax`命令を追加しEAX(システムコール番号)が破壊される事象が
# 発生した為、システムコール入口の処理は純アセンブリに書き換えた
.intel_syntax noprefix
.global syscall_handler

syscall_handler:
    pusha

    ## 確認用：VRAMに 'B' を出す
    # mov edi, 0xB8002
    # mov byte ptr [edi], 'B'
    # mov byte ptr [edi+1], 0x0F
    
    # syscall番号分岐
    cmp eax, 1
    je syscall_write

    cmp eax, 2
    je syscall_read_sector
    
    cmp eax, 3
    je syscall_fs_open

    cmp eax, 4
    je syscall_fs_read

    cmp eax, 5
    je syscall_fs_close
    
syscall_write:
    push ebx                # 引数（文字列ポインタ）
    call handle_write
    add esp, 4
    jmp syscall_done
    
syscall_read_sector:
    push ecx        # buffer
    push ebx        # lba
    call handle_read_sector
    add esp, 8
    #返り値はEAXの中(成功コード0)
    jmp syscall_done
    
syscall_fs_open:
    push ebx                # filename
    call handle_fs_open
    add esp, 4
    jmp syscall_done

syscall_fs_read:
    push edx                # size
    push ecx                # buf
    push ebx                # fd
    call handle_fs_read
    add esp, 12
    jmp syscall_done

syscall_fs_close:
    push ebx                # fd
    call handle_fs_close
    add esp, 4
    jmp syscall_done

syscall_done:
    popa
    iret

