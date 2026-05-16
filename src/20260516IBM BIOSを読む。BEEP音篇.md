# 初めに
以下のIBMBIOSで実装されているビープ音の処理を追いました。
https://github.com/gawlas/IBM-PC-BIOS/blob/master/IBM%20PC/PCBIOSV3.ASM

NASMでコンパイル出来るように文法を少し直して、注釈を追記し、ブートローダから実行出来るように少し手直ししました。

プログラム中で直接制御しているタイマは以下のチップです。
http://www.itofamily.com/ito/collections/peripherals/i8253-54/
https://www.toolify.ai/ja/hardwarejp/82538254%E5%8F%AF%E8%83%BD%E9%81%95-2978375

短いので抜粋すると以下の様になっており、ビープ音を鳴らす処理が書かれています。

呼び出し例:`ERR_BEEP`の中
```
	MOV	BL,6			; COUNTER FOR BEEPS
	CALL	BEEP			; DO THE BEEP
```

```
TIMER		EQU	40H
PORT_B		EQU	61H		; 8255 PORT B ADDR

; ...(省略)...

BEEP	PROC	NEAR
	MOV	AL,10110110B	; SEL TIM 2,LSB,MSB,BINARY
	OUT	TIMER+3,AL		; WRITE THE TIMER MODE REG
	MOV	AX,533H 		; DIVISOR FOR 1000 HZ
	OUT	TIMER+2,AL		; WRITE TIMER 2 CNT - LSB
	MOV	AL,AH
	OUT	TIMER+2,AL		; WRITE TIMER 2 CNT - MSB
	IN	AL,PORT_B		; GET CURRENT SETTING OF PORT
	MOV	AH,AL			; SAVE THAT SETTING
	OR	AL,03			; TURN SPEAKER ON
	OUT	PORT_B,AL
	SUB	CX,CX			; SET CNT TO WAIT 500 MS
G7:
	LOOP	G7			; DELAY BEFORE TURNING OFF
	DEC	BL			; DELAY CNT EXPIRED?
	JNZ	G7			; NO - CONTINUE BEEPING SPK
	MOV	AL,AH			; RECOVER VALUE OF PORT
	OUT	PORT_B,AL
	RET				; RETURN TO CALLER
BEEP	ENDP
```

# 検証

VRAMには何も書き込んでいないので、何も表示されていません。
この状態で音が鳴り続けました。
どのくらい鳴り続けるか見たかったのですが、結構な音量で近所迷惑となるので断念しました。
<img width="2359" height="1769" alt="62ebed97634568" src="https://github.com/user-attachments/assets/4ecb59f7-ad0a-4bb0-b830-011acbf1f0fe" />

隣から毎日ピアノの練習の音が聞こえてくるのでこれくらい良いかも知れませんが外国にいるのでいざこざは避けるが吉です。


# 修正コード

パソコン上で検証出来るようにブートローダに書き直したのが以下のコードです。

```boot.asm
[org 0x7C00]
bits 16

start:

mov bl, 6
call beep
hlt

beep:

    ;-------------------------
    ; PIT Timer2 設定
    ;-------------------------
    mov al, 10110110b
    out 0x43, al            ; 制御レジスタ: 10=ch2 11=上位下位書き込み  011=mode3 0BCD

    mov ax, 0x0533          ; 1.19318MZ / 0x533≒1000Hz 1kHzくらいの音を出すための分周値

    out 0x42, al            ; 下位バイト
    mov al, ah
    out 0x42, al            ; 上位バイト
    ;タイマ2が約1kHzで刻み始める

    ;-------------------------
    ; speaker 入
    ;-------------------------
    ;bit0-1だけを`11`にしたい　bit0:スピーカ信号ON　bit1:8253の出力を通す
    in al, 0x61
    mov ah, al
    or al, 0x03
    out 0x61, al

    ;-------------------------
    ; 遅延
    ;-------------------------
    sub cx, cx

.delay:
    loop .delay

    dec bl
    jnz .delay

    ;-------------------------
    ; speaker 切（元に戻す）
    ;-------------------------
    mov al, ah
    out 0x61, al

    ret

times 510-($-$$) db 0
dw 0xAA55
```

以下はバグでしょうか？これを実行するとかなり長くループが回り、`BL`を設定した意味が無いように感じます。
しかし天下のIBMがこんな間違いを放置する訳無いと思うので何か意図があるのでしょう。
```
    sub cx, cx

.delay:
    loop .delay
```


