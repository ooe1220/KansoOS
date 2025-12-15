[BITS 32]
global isr6
extern exception_handler

isr6:
    pusha
    cli
    call exception_handler
    sti
    popa
    iret
