[BITS 16]
[ORG 0x7E00]

start:
    cli                     ; 割り込み禁止

    ; ----------------------------
    ; VGA グラフィックモード設定
    ; mode 13h : 320x200 256色
    ; ----------------------------
    mov ax, 0x0013
    int 0x10

    ; ----------------------------
    ; VRAM (A000:0000) を赤で塗る
    ; ----------------------------
    mov ax, 0xA000
    mov es, ax
    xor di, di               ; DI = 0

    mov al, 4                ; 色番号 4 = 赤
    mov cx, 320*200          ; 64000 バイト

.fill:
    stosb                    ; ES:DI ← AL, DI++
    loop .fill
    
    
; VGAレジスタを設定してMODE3を設定
%include "1MiscellaneousOutput.asm"
%include "2Sequencer.asm"
%include "3crtc.asm"
%include "4GraphicsController.asm"
%include "5AttributeController.asm"

mov ax, 0xB800
mov es, ax
mov word [es:0],  0x0441   ; 'A' 赤
mov word [es:2],  0x0142   ; 'B' 青

.halt:
    hlt
    jmp .halt                ; 完全停止
