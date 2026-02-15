.intel_syntax noprefix
.global user_exec
.global user_exec_ret

/* kernelスタック退避用 */
.section .bss
.global kernel_esp_saved
kernel_esp_saved:
    .space 4          /* resd 1 相当 */

.global kernel_ebp_saved
kernel_ebp_saved:
    .space 4

.section .text

/* void* entry, int argc, char** argv */
user_exec:

    /* カーネルスタック退避 */
    //mov DWORD PTR [kernel_esp_saved],esp  
    //mov DWORD PTR [kernel_ebp_saved],ebp

    //ret
    //hlt
    /* ESPに渡された引数を取得 (cdecl, 32bit) */
    /* [esp]   = return address */
    /* [esp+4] = entry */
    /* [esp+8] = argc */
    /* [esp+12]= argv */
    mov eax, DWORD PTR [esp+4]   /* entry */
    mov esi, DWORD PTR [esp+8]   /* argc */
    mov edi, DWORD PTR [esp+12]  /* argv */
    
    /* ユーザースタックに切替 */
    //mov esp, 0x30000
    
    /* ユーザスタックに argc/argv を push */
    push edi   /* argv */
    push esi   /* argc */
    
    /* ユーザプログラムへ飛ぶ */
    jmp eax

/* exit から戻るラベル */
user_exec_ret:
    add esp,8
    //mov esp, DWORD PTR [kernel_esp_saved]  /* カーネルスタック復元 */
    //mov ebp, DWORD PTR [kernel_ebp_saved]
    //hlt
    ret

