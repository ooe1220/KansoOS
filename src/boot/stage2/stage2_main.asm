; カーネルを起動する前に必要な初期化を行う。

; メモリマップ
; 0x7C00  : VBR/CDBOOT
; 0x8000  : このSTAGE2本体
; 0x18000 : kernel.bin（呼び出し元がここへ読み込み済み）
; 0x200000: kernel.binへ跳ぶ

%define KERNEL_TMP_ADDR    0x18000      ; カーネル一時置き場
%define KERNEL_LOAD_ADDR   0x200000   ; カーネル配置領域
%define KERNEL_REGION_SIZE    0x100000 ; kernel.bin + bssの為に予約した領域
%define KERNEL_FILE_SIZE   0x10000          ; kernel.binの大きさ 64KB

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

    call keybord_init
    call cpuid_check
    call a20_enable
    call pic_init

    call real_to_prot; 32bitモードへ切り替え

bits 32

   cld

   ; カーネル領域を0で埋める（BSSもここで一気に0埋め）
    mov edi, KERNEL_LOAD_ADDR
    mov ecx, KERNEL_REGION_SIZE / 4 ; DWORD
    xor eax, eax
    rep stosd               ; [EDI] <- EAX

    ; ブートセクタから0x18000に置いたkernel.binを0x200000へ複製 
    ; 16bitモードでは0x200000へ届かない為ここで複製する
    ; カーネルを1MB以内に置くと今後肥大化した際にVRAMやBOISを侵食し兼ねない
    mov esi, KERNEL_TMP_ADDR
    mov edi, KERNEL_LOAD_ADDR 
    mov ecx, KERNEL_FILE_SIZE / 4           ; 64KB / 4 (DWORD)
    rep movsd

    call checksum ; kernel.binがビット反転していないか確認

    jmp prot_to_real ; VBEを設定する為に16ビットモードへ戻る 32bitアドレスをCS:IPにするのが難しく一旦JMP

bits 16
prot_to_real_end: 

    call vbe_init ; VBE 800*600*16bpp設定
    
    call real_to_prot;
bits 32

    ; ここからkenelへ飛ぶ
    jmp KERNEL_LOAD_ADDR

;--------------------------------------------------------LIB-------------------------------------------------------
; 必ず各ファイルの初めにbits 16/32をつけること
%include "src/boot/stage2/cpuid.asm" 
%include "src/boot/stage2/pm.asm" 
%include "src/boot/stage2/a20.asm"
%include "src/boot/stage2/pic_init.asm"
%include "src/boot/stage2/keybord_init.asm"  
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