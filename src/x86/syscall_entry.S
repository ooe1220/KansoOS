# 初めはC言語中にインラインアセンブラとして実装していたが、
# C言語をコンパイルする際にPIC(アドレス間接参照)の影響により
# コンパイラが勝手に`add $0x1,%eax`命令を追加しEAX(システムコール番号)が破壊される事象が
# 発生した為、システムコール入口の処理は純アセンブリに書き換えた
#include "syscall_num.h" 
.intel_syntax noprefix
.global syscall_handler

syscall_handler:
    pusha

    ## 確認用：VRAMに 'B' を出す
    # mov edi, 0xB8002
    # mov byte ptr [edi], 'B'
    # mov byte ptr [edi+1], 0x0F
    
    # syscall番号分岐
    cmp eax, SYS_WRITE
    je syscall_write

    cmp eax, SYS_READ_SECT
    je syscall_read_sector
    
    cmp eax, SYS_FS_OPEN
    je syscall_fs_open

    cmp eax, SYS_FS_READ
    je syscall_fs_read

    cmp eax, SYS_FS_CLOSE
    je syscall_fs_close
    
    cmp eax, SYS_UMALLOC
    je syscall_umalloc

    cmp eax, SYS_UFREE
    je syscall_ufree

    cmp eax, SYS_GFX_FLIP
    je syscall_gfx_flip

    cmp eax, SYS_GETCHAR
    je syscall_getchar

    
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
    
    
syscall_umalloc:
    push ebx                # size
    call handle_umalloc
    add esp, 4
    jmp syscall_done

syscall_ufree:
    push ebx                # ptr
    call handle_ufree
    add esp, 4
    jmp syscall_done

syscall_gfx_flip:
    push ebx                # EBX = &g_gfx （ユーザ側の struct gfx_buf*）
    call handle_gfx_flip
    add esp, 4
    jmp syscall_done

syscall_getchar:               # 引数なしなので push 不要
    call handle_getchar
    jmp syscall_done

syscall_done:
    mov [esp+28], eax # pushaでスタックに積んだEAXを返り値に置き換える。そうしないとpopaで返り値が壊れる
    popa
    iret

