.intel_syntax noprefix
.global _start
.extern main

_start:
    //call main           # Cのmainを呼ぶ
    call label
label:
    //pop eax
    //push eax
    //hlt
    jmp main

    # exit syscall
    mov eax, 0          # syscall番号 0 = exit
    int 0x80

