# 初めはC言語中にインラインアセンブラとして実装していたが、
# C言語をコンパイルする際にPIC(アドレス間接参照)の影響により
# コンパイラが勝手に`add $0x1,%eax`命令を追加しEAX(システムコール番号)が破壊される事象が
# 発生した為、システムコール入口の処理は純アセンブリに書き換えた
.intel_syntax noprefix
.global syscall_handler

syscall_handler:
    pusha

    ## 確認用：VRAMに 'B' を出す
    #mov edi, 0xB8002
    #mov byte ptr [edi], 'B'
    #mov byte ptr [edi+1], 0x0F
    cmp eax, 1              # syscall番号分岐
    jne syscall_done

    push ebx                # 引数（文字列ポインタ）
    call handle_write
    add esp, 4

syscall_done:
    popa
    iret

