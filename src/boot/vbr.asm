[BITS 16]
[ORG 0x7C00]

; FAT16のBPB
jmp start
nop

; --- OEM名（8バイト） ---
db "KANSO OS"     ; OEM名(8bitに満たない場合は空白埋め)

; --- BIOS パラメータブロック (BPB) ---
dw 512            ; 1セクタあたりのバイト数
db 8              ; クラスタあたりのセクタ数 (4KB×512B=4KB)
dw 1              ; 予約セクタ数 (VBR 自身の1セクタ)
db 2              ; FATテーブル数
dw 512            ; ルートディレクトリ最大要素エントリ数
dw 0              ; 総セクタ数16bit（0にして32bitを使用）
db 0xF8           ; 記憶媒体の種類 HDD固定
dw 8              ; 各FATのセクタ数
dw 63             ; トラックあたりのセクタ数
dw 255            ; ヘッド数
dd 0              ; 隠しセクタ数
dd 2048           ; 総セクタ数32bit（1MB / 512 = 2048セクタ）

; --- 拡張BPB ---
db 0x80           ; BIOSドライブ番号（0x80 = HDD）
db 0              ; 予約領域（使用しない）
db 0x29           ; 拡張ブート用
dd 0x12345678     ; ボリュームシリアル番号

; --- ボリュームラベル（11バイト） ---
db "FAT16DISK  "  ; 空白で11バイトに調整

; --- ファイルシステムの種類（8バイト） ---
db "FAT16   "     ; 空白で8バイトに調整


start:

    mov si, msg_loaded
    call print_string

    ; kernelは64セクタ分(32KB)，LBA=126
    ; KERNEL.BIN は LBA 126 セクタ目から始まる
    mov ah, 0x02
    mov al, 63 ; 読み込みセクタ数 1トラック分（32KB）
    mov ch, 0 ; シリンダ
    mov cl, 1 ; セクタ
    mov dh, 2 ; ヘッド
    mov dl, 0x80 ; HDD
    mov bx, 0x8000 ; メモリ0x8000番地へ読み込む
    int 0x13
    jc load_error ; 

    jmp 0x0000:0x8000 ; kernelの開始アドレスへ跳ぶ

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret
    
load_error:
    mov si, error_msg
    call print_string

msg_loaded db "[VBR] Execution started at 0x0000:0x7C00", 0x0D, 0x0A, 0
error_msg db "Failed to load KERNEL.BIN", 0x0D, 0x0A, 0

; 残りの領域を512バイトまで埋める
times 510-($-$$) db 0

dw 0xAA55

