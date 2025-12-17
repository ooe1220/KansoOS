[BITS 32]
global isr6
extern exception_handler

isr6:
    call exception_handler
    iret
