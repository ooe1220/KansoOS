; VBRもしくはCDBOOTから呼ばれる。
; カーネルを起動する前に必要な初期化を行う。

bits 16
org 0x8000 ; 0x0000 : 0x8000

    jmp start
    vbe_info: dd 0 ; modeinfoの先頭アドレス

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

post_start:

    mov si, test_msg
    call print_string

; ここにPOST処理を入れる

    ; VBE対応しているか
    mov ax, 0x4F00
    mov di, controllerinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; VBEのVRAMをBIOSに聞く
    mov ax, 0x4F01
    mov cx, 0x114 ; mov cx, 0x115          ; 聞きたいモード
    mov di, modeinfo
    int 0x10
    cmp ax, 0x004F
    jne no_vbe
    
    ; 800x600
    mov ax, 0x4F02    ; VBE Set Mode
    mov bx, 0x4114 ; mov bx, 0x4115
    int 0x10
    cmp ax, 0x004F
    jne no_vbe

    ; vbe_infoへVBE情報構造体のアドレスを置く
    mov eax, modeinfo
    mov [vbe_info], eax

jmp enter_pm ; 32bitモードへ切り替え

no_vbe:
    mov si, error_msg
    call print_string
    ret

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; BIOSが返すVBEの構造体255バイト開始アドレス
controllerinfo: times 0x100 db 0 ; こちらは使わないが受け皿
modeinfo: times 0x100 db 0

test_msg db "Custom POST started", 0x0D, 0x0A, 0
error_msg db "FVBE Error", 0x0D, 0x0A, 0

%include "src/boot/pm.asm" ; 注意、下に置くと32ビットコードに変換され不具合
;-------------------------------------------------------32-------------------------------------------------------

bits 32
enter_pm_return:

    ; ブートセクタから0x18000に置いたkernel.binを0x200000へ複製 
    ; 64KBの複製は2回に分けないと実機で失敗した
    cld
    mov esi, 0x18000
    mov edi, 0x200000 
    mov ecx, 8192           ; 32KB / 4 (DWORD)
    rep movsd

    mov esi, 0x20000         ; 0x18000 + 32KB
    mov edi, 0x208000       ; 0x200000 + 32KB
    mov ecx, 8192
    rep movsd

    ; ここからkenelへ飛ぶ
    jmp 0x200000

; ---- ここから下：ファイルをちょうど64KBにする ----
times 0x10000 - ($ - $$) db 0