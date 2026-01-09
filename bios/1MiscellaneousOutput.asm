; --- Miscellaneous Output Register ---
; bit7: 1 = クロック 28MHz, bit6: 1 = 未使用
; bit5: 1 = RAMモード 256KB
; bit4: 0 = モニタタイプ
; bit3-0: 0111 = I/O/Memory, クロック選択

mov dx, 0x3C2 ; 0110 0111
mov al, 0x67
out dx, al
