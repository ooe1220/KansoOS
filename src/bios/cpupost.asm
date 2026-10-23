; IBMBIOS 参考

cpupost:
;------------------------------
;   フラグレジスタの診断
;------------------------------
	cli           ; 割り込み禁止
	mov ah,0xd5   ; AH=11010101 SF,ZF,AF,PF,CFを1に設定
	sahf          ; flags←AH
	
	;CF,ZF,PF,SF検証
	jnc error1    ; CF!=1→ERR
	jnz error1    ; ZF!=1→ERR
	jnp error1    ; PF!=1→ERR
	jns error1    ; SF!=1→ERR
	
	; AF検証(AF専用分岐は存在しない)
	lahf          ; AH←flags
	mov cl,5
	shr ah,cl     ; AH = 110`1`0101b 5回右にずらし1が落ちたらCF=1 即ちAF=CF
	jnc error1    ; CF(AF)!=1→ERR
	
	; OF検証(SAHFはflags下位8ビット分しか設定できない。11ビット目のOFは直接1に出来ない。)
	mov al, 0x40  ; AL=01000000b
	shl al, 1     ; 左に1ずらす→10000000b 01000000b (+64)→10000000b (-128)符号付演算の異常でOF=1となる
	jno error1
	
	; 次はフラグレジスタをに0に設定して検証
	xor ah, ah
	sahf          ; フラグレジスタ下位8ビットを全て0に
	jbe error1    ; CF==1 or ZF==1→ERR
	js error1     ; SF==1→ERR
	jp error1     ; PF==1→ERR
	
	; AF検証
	lahf
	mov cl,5
	shr ah,cl
	jc error1     ; CF==1→ERR
	
	; OF検証
	shl ah,1
	jo error1     ;  OF==1→ERR
	
;------------------------------
;   汎用及びセグメントレジスタの診断
;------------------------------
	mov ax, 0xffff
	stc             ; CF=1設定
c8:
	mov ds, ax
	mov bx, ds
	mov es, bx
	mov cx, es
	mov ss, cx
	mov dx, ss
	mov sp, dx
	mov bp, sp
	mov si, bp
	mov di, si
	
	jnc c9         ; CFで 1回目(FFFF) / 2回目(0000)を区別 2回終わったら終了
	xor ax, di
	jnz error1     ; AX(0xFFFF)!=DI(0xFFFF)ならどこかのレジスタ演算が壊れている
	clc            ; CF=0設定
	jmp c8         ; 2度目は0x0000で再確認(XOR命令でAX=0x0000)

c9:
	xor ax, di     ; AX(0x0000)==DI(0x0000)か？
	jz c10         ; 次の検証
	
error1: 
	mov ax, 0xB800
	mov es, ax
	mov di, 0x0F9C ; VRAM最後から2文字分
	mov ah, 0x0C ; 赤
	mov al, 'E'
	mov [es:di], ax
	add di,2
	mov al, 'R'
	mov [es:di], ax
	add di,2
	hlt        ; CPU停止

c10: 
	ret
