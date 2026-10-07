bits 32
global start
extern kmain

align 4
mb_header:
    dd 0x1BADB002        ; magic
    dd 0x0               ; flags
    dd -(0x1BADB002 + 0x0) ; checksum

start:
    cli
    mov esp, 0x90000     ; 適当なスタック
    call kmain
    hlt

