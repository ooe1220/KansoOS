
;----------------------------------------
; Sequencer Registers
;----------------------------------------
; index 0
; bit1=1 CPUからのデータ受付を開始
; bit2=1 画面への信号送信を開始
; bit3-7 未使用
mov dx, 0x3C4
mov al, 0x00
out dx, al
mov dx, 0x3C5
mov al, 0x03         ; 0000 0011
out dx, al


; index 1
; bit0=0 1文字の横幅は8ドット(1は9ドット)
; bit1=0 未使用
; bit2=0 
; bit3=0 ドットクロック等倍
; bit4=0 
; bit5=0 画面表示,1にすると画面が暗くなる
; bit6-7=0 未使用
mov dx, 0x3C4
mov al, 0x01
out dx, al
mov dx, 0x3C5
mov al, 0x00         ; 0000 0000
out dx, al


; index 2
; bit0=1 プレーン0への書き込み許可
; bit1=1 プレーン1への書き込み許可
; bit2=0 プレーン2への書き込み禁止
; bit3=0 プレーン3への書き込み禁止
; bit4-7=0 未使用
mov dx, 0x3C4
mov al, 0x02
out dx, al
mov dx, 0x3C5
mov al, 0x03         ; 0000 0011
out dx, al


; index 3
; bit0, 1, 4=000 1番目の字体を使う
; bit2, 3, 5=000 1番目の字体を使う(ここで英語以外の字体を設定したりできる。今回は無し)
; bit6-7 = 0
mov dx, 0x3C4
mov al, 0x03         ; index 3 : Character Map Select
out dx, al
mov dx, 0x3C5
mov al, 0x00         ; font from plane 0
out dx, al


; index 4
; bit0 = 0 : 拡張メモリ（未使用）
; bit1 = 1 : 偶数・奇数モード有効（モード3では必須）
; bit2 = 0 : Chain-4モード無効
; bit3-7 = 0 : 未使用
mov dx, 0x3C4
mov al, 0x04         ; index 4 : Memory Mode
out dx, al
mov dx, 0x3C5
mov al, 0x02         ; 00000010
out dx, al

