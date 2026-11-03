; VBE レジスタへ (index, value) を書き込む
%macro vbe_write 2
    mov     dx, 0x01CE
    mov     ax, %1
    out     dx, ax
    mov     dx, 0x01CF
    mov     ax, %2
    out     dx, ax
%endmacro

vbe_800_600_16_init:

    ; 0x3C0 のフリップフロップを初期化
    mov     dx, 0x3C0
    mov     al, 0x20
    out     dx, al

    vbe_write 0x0004, 0x0000   ; 画面出力を一旦停止
    vbe_write 0x0001, 800      ; 横
    vbe_write 0x0002, 600      ; 縦
    vbe_write 0x0003, 16       ; bpp
    vbe_write 0x0004, 0x00C1   ; 画面出力再開

ret