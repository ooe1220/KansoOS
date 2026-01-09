; --- Miscellaneous Output Register ---
mov al, 0x67       ; 0b01100111
                    ; bit7: 1 = クロック 28MHz, bit6: 1 = 未使用
                    ; bit5: 1 = RAMモード 256KB
                    ; bit4: 0 = モニタタイプ
                    ; bit3-0: 0111 = I/O/Memory, クロック選択
out 0x3C2, al      ; 書き込みポート
