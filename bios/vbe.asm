
vbe_800_600_16_init:

    mov dx, 0x3C0
    mov al, 0x20
    out dx, al

    mov dx, 0x01CE
    mov ax, 0x0004
    out dx, ax
    mov dx, 0x01CF
    mov ax, 0x0000
    out dx, ax

    ;800
    mov dx, 0x01CE
    mov ax, 0x0001
    out dx, ax
    mov dx, 0x01CF
    mov ax, 800
    out dx, ax

    ;600
    mov dx, 0x01CE
    mov ax, 0x0002
    out dx, ax
    mov dx, 0x01CF
    mov ax, 600
    out dx, ax

    ;16
    mov dx, 0x01CE
    mov ax, 0x0003
    out dx, ax
    mov dx, 0x01CF
    mov ax, 16
    out dx, ax

    mov dx, 0x01CE
    mov ax, 0x0004
    out dx, ax
    mov dx, 0x01CF
    mov ax, 0x00C1
    out dx, ax

ret

