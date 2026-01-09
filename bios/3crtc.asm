;----------------------------------------
; CRTC Registers
;----------------------------------------

; 書き込み許可(0x11のbit7=1の間はCRTCレジスタ 0x00〜0x07の書き換えが禁止されている)
mov dx, 0x3D4
mov al, 0x11
out dx, al
mov dx, 0x3D5
mov al, 0x0E                ; bit7=0 unlock
out dx, al

; 0x00 水平方向の表示幅の合計
; 1行の走査線には100何文字ある
; 「実際の文字数-5」の値を入れる 
mov dx, 0x3D4
mov al, 0x00
out dx, al
mov dx, 0x3D5
mov al, 0x5F ; 95
out dx, al

; 0x01 水平方向に実際に表示する幅
; 「表示したい文字数 - 1」 の値を入れる 1行に80文字表示
mov dx, 0x3D4
mov al, 0x01
out dx, al
mov dx, 0x3D5
mov al, 0x4F ; 79
out dx, al

; 0x02 水平帰線消去開始
; 電子銃が画面の右端から左端へ戻る際に、画面に余計な線が出ないように信号を消す処理
; 79文字目まで表示、80文字目から電子銃の電子発射を止める
mov dx, 0x3D4
mov al, 0x02
out dx, al
mov dx, 0x3D5
mov al, 0x50 ; 80
out dx, al

; 0x03 水平帰線消去終了
; 0x82=1000 0010
; bit 7	互換性ビット　1 固定
; bit 5-6 表示のズレ補正　00（補正なし。標準の設定）
; bit 0-4 終了位置の数値(新行の2文字目から電子発射再開) 00010 (2)
mov dx, 0x3D4
mov al, 0x03
out dx, al
mov dx, 0x3D5
mov al, 0x82
out dx, al

; 0x04 水平同期信号開始
; 85文字目から電子銃を左側に戻す
mov dx, 0x3D4
mov al, 0x04
out dx, al
mov dx, 0x3D5
mov al, 0x55 ; 85
out dx, al

; 0x05 Horizontal Retrace End
mov dx, 0x3D4
mov al, 0x05
out dx, al
mov dx, 0x3D5
mov al, 0x81 ; 1000 0001
out dx, al

; 0x06 垂直総行数 下位8ビット分
mov dx, 0x3D4
mov al, 0x06
out dx, al
mov dx, 0x3D5
mov al, 0xBF ; 191
out dx, al

; 0x07 Overflow
; bit0=1 縦の行数=191+256=447
; bit 0	0x06 (Vertical Total)	9ビット目
; bit 1	0x12 (Vertical Display End)	9ビット目
; bit 2	0x10 (Vertical Retrace Start)	9ビット目
; bit 3	0x15 (Vertical Blank Start)	9ビット目
; bit 4	0x18 (Line Compare)	9ビット目
; bit 5	0x06 (Vertical Total)	10ビット目
; bit 6	0x12 (Vertical Display End)	10ビット目
; bit 7	0x10 (Vertical Retrace Start)	10ビット目
mov dx, 0x3D4
mov al, 0x07
out dx, al
mov dx, 0x3D5
mov al, 0x1F ; 0001 1111
out dx, al

; 0x08 画面の一番上の行を、文字の何ドット目から描き始めるか
; 例えば2に設定すると文字の頭2ドット分が見えなくなる
mov dx, 0x3D4
mov al, 0x08
out dx, al
mov dx, 0x3D5
mov al, 0x00 ; 0
out dx, al

; 0x09 1文字（1行）の高さを、縦に何ドット分にするか
bit 7	0	2倍スキャン	1行を2回描いて縦に引き伸ばすか？ → しない（通常表示）
bit 6	1	画面分割用の桁	「ラインコンペア」という機能で使う数字の一部（9ビット目）
bit 5	0	消去開始用の桁	「垂直帰線消去開始」で使う数字の一部（9ビット目）
bit 0-4	01111 (15)	1文字の高さ	文字を縦何ドットで描くか。設定値+1が実数値 → 16ドット
mov dx, 0x3D4
mov al, 0x09
out dx, al
mov dx, 0x3D5
mov al, 0x4F ; 0100 1111
out dx, al

; 0x0A カーソル開始行
; bit 0-4 : 開始行指定 1文字の枠内（縦0〜15行）のうち、13行目から塗り潰し始める。
; bit 5 : カーソルON/OFF。 0 で表示、1 で非表示（透明）になります。
; bit 6-7 : 未使用。
mov dx, 0x3D4
mov al, 0x0A
out dx, al
mov dx, 0x3D5
mov al, 0x0D ; 00001101
out dx, al

; 0x0B カーソル終了行
; bit 0-4 (値: 0x0E) : 終了ライン指定。 14行目で塗り潰しを終了
; bit 5-6 (値: 00) : 表示のズレ（Skew）。 文字に対してカーソルを右に何ドットずらすかを決める。00 はズレなし。
; bit 7 : 未使用。
mov dx, 0x3D4
mov al, 0x0B
out dx, al
mov dx, 0x3D5
mov al, 0x0E ; 00001110
out dx, al

; 0x0C 表示開始アドレス　上位8ビット
mov dx, 0x3D4
mov al, 0x0C
out dx, al
mov dx, 0x3D5
mov al, 0x00
out dx, al

; 0x0D 表示開始アドレス　下位8ビット
; 上位下位合わせて0x0000. 即ちVRAMの先頭から表示する
mov dx, 0x3D4
mov al, 0x0D
out dx, al
mov dx, 0x3D5
mov al, 0x00
out dx, al

; 0x0E カーソルを画面のどこに置くか　上位8ビット　　　０２
mov dx, 0x3D4
mov al, 0x0E
out dx, al
mov dx, 0x3D5
mov al, 0x00
out dx, al

; 0x0F カーソルを画面のどこに置くか　下位8ビット　　　AD
; 注意：座標では無く左上から数えて何文字目か
; 例
; 0 を指定した場合：画面の左上端（1行目の1文字目）にカーソルが出る
; 80 を指定した場合：1行が80文字なら、2行目の1文字目にカーソルが出る
mov dx, 0x3D4
mov al, 0x0F
out dx, al
mov dx, 0x3D5
mov al, 0x00
out dx, al

; 0x10 Vertical Retrace Start
mov dx, 0x3D4
mov al, 0x10
out dx, al
mov dx, 0x3D5
mov al, 0x9C
out dx, al

; 0x11 Vertical Retrace End　　　　８E
mov dx, 0x3D4
mov al, 0x11
out dx, al
mov dx, 0x3D5
mov al, 0x0E
out dx, al

; 0x12 Vertical Display End
mov dx, 0x3D4
mov al, 0x12
out dx, al
mov dx, 0x3D5
mov al, 0x8F
out dx, al

; 0x13 Offset
mov dx, 0x3D4
mov al, 0x13
out dx, al
mov dx, 0x3D5
mov al, 0x28
out dx, al

; 0x14 Underline Location　　　１f
mov dx, 0x3D4
mov al, 0x14
out dx, al
mov dx, 0x3D5
mov al, 0x00
out dx, al

; 0x15 Vertical Blank Start
mov dx, 0x3D4
mov al, 0x15
out dx, al
mov dx, 0x3D5
mov al, 0x96
out dx, al

; 0x16 Vertical Blank End
mov dx, 0x3D4
mov al, 0x16
out dx, al
mov dx, 0x3D5
mov al, 0xB9
out dx, al

; 0x17 Mode Control
mov dx, 0x3D4
mov al, 0x17
out dx, al
mov dx, 0x3D5
mov al, 0xA3
out dx, al

; 0x18 Line Compare
mov dx, 0x3D4
mov al, 0x18
out dx, al
mov dx, 0x3D5
mov al, 0xFF
out dx, al





