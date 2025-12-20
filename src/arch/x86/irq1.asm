[BITS 32]
global irq1
extern keyboard_handler

irq1:
    pusha
    call keyboard_handler
    popa
    iretd

