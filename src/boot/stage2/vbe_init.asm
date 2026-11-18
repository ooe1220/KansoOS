bits 16

vbe_init:

    ; VBE対応しているか
    mov ax, 0x4F00
    mov di, controllerinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; VBEの情報をBIOSに聞く
    mov ax, 0x4F01
    mov cx, 0x114 ; 800x600 16bpp
    mov di, modeinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; 800x600
    mov ax, 0x4F02    ; VBE Set Mode
    mov bx, 0x4114    ; 800x600 16bpp
    int 0x10
    cmp ax, 0x004F
    jne no_vbe

    ; vbe_infoへVBE情報構造体のアドレスを置く
    mov eax, modeinfo
    mov [vbe_info], eax

    ret

no_vbe:
.hang:
    cli
    hlt
    jmp .hang 

