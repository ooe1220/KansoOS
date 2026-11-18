; カーネルを起動する前に必要な初期化を行う。

; メモリマップ
; 0x7C00  : VBR/CDBOOT
; 0x8000  : このSTAGE2本体
; 0x18000 : kernel.bin（呼び出し元がここへ読み込み済み）
; 0x200000: kernel.binへ跳ぶ

bits 16
org 0x8000 ;  0x0000:0x8000 に読み込まれている前提（VBR/CDBOOTがここへ読み込む)

    jmp start
    vbe_info: dd 0 ; modeinfoの先頭アドレス 後続のカーネル(C言語)より参照する
    expected_sum:   dd 0x00000000 ; kernel.binのチェックサム値。ビルドスクリプトから入れる

align 4  ; jmp からstartとの間にdd一個だと動くが、dd2つ置くならこれを入れないとその後正常に動作しなくなる。
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00 ; 仮スタック

    mov si, msg_post_start
    call print_string

    ; ここに少しずつBIOSを真似たPOST処理を追加して行きたい

    call cpuid_check
    call a20_enable
    call vbe_init ; VBE 800*600*16bpp設定 これ移行カーネルのconsoleが動くまでは文字表示不可

    call enter_pm; 32bitモードへ切り替え(モードを切り替える為call retは使わない)

;-------------------------------------------------------32-------------------------------------------------------

bits 32

    ; ブートセクタから0x18000に置いたkernel.binを0x200000へ複製 
    ; 16bitモードでは0x200000へ届かない為ここで複製する
    ; カーネルを1MB以内に置くと今後肥大化した際にVRAMやBOISを侵食し兼ねない
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

    call checksum ; kernel.binがビット反転していないか確認

    ; ここからkenelへ飛ぶ
    jmp 0x200000

;--------------------------------------------------------LIB-------------------------------------------------------
; 必ず各ファイルの初めにbits 16/32をつけること
%include "src/boot/stage2/cpuid.asm" 
%include "src/boot/stage2/pm.asm" 
%include "src/boot/stage2/a20.asm" 
%include "src/boot/stage2/vbe_init.asm" 
%include "src/boot/stage2/lib.asm" 
%include "src/boot/stage2/checksum.asm" 

;------------------------------------------------------DATA------------------------------------------------------

msg_post_start db "Custom POST started", 0x0D, 0x0A, 0
error_msg db "FVBE Error", 0x0D, 0x0A, 0

; BIOSが返すVBEの構造体255バイト開始アドレス
controllerinfo: times 0x100 db 0 ; こちらは使わないが受け皿
modeinfo: times 0x100 db 0

; ---- 64KBに合わす----
times 0x10000 - ($ - $$) db 0