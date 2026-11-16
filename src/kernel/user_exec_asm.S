#include "lib/memmap.h"
// argc: 引数個数, argv: 引数配列
// 引数を指定しない場合、argc=1, argv[0]=ファイル名
.global user_exec
    .type   user_exec, @function
user_exec:
    push    %ebp
    mov     %esp, %ebp
    push    %ebx
    push    %esi
    push    %edi

    /* カーネル用ESP,EBPを静的領域へ保存 */
    mov     %esp, saved_kernel_esp
    mov     %ebp, saved_kernel_ebp

    /* 引数をレジスタへ（呼び出し規約: [ebp+8]=entry,+12=argc,+16=argv） */
    mov     0x8(%ebp),  %ebx      /* entry */
    mov     0xc(%ebp),  %esi      /* argc  */
    mov     0x10(%ebp), %edi      /* argv  */

    /* ユーザースタックへ切替 */
    mov     $USER_STACK_TOP, %esp
    xor     %ebp, %ebp

    /* cdecl: 引数を逆順でプッシュ */
    push    %edi                  /* argv */
    push    %esi                  /* argc */
    call    *%ebx

    /* 引数解放 + カーネルESP,EBP復元 */
    add     $8, %esp
    mov     saved_kernel_esp, %esp
    mov     saved_kernel_ebp, %ebp

    pop     %edi
    pop     %esi
    pop     %ebx
    pop     %ebp
    ret
    .size   user_exec, .-user_exec

    .section .bss
    .global saved_kernel_esp
    .global saved_kernel_ebp
    .align 4
saved_kernel_esp:
    .long 0
saved_kernel_ebp:
    .long 0