bits 32
global start
extern kernel_main

start:

    mov esp, 0x900000

    mov dx, 0x3F8
    mov al, 'K'
    out dx, al
            
    call kernel_main ; kernel.cへ移行

.hlt_loop:
    hlt
    jmp .hlt_loop

; objdump -b binary -m i8086 -D build/kernel.bin | head -60

