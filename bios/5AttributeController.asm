;========================================
; Attribute Controller 全レジスタ設定
; 0x00–0x14 (21 registers)
;========================================

mov dx, 0x3DA
in  al, dx              ; flip-flop reset

mov dx, 0x3C0

;----------------------------------------
; Palette Registers 0x00–0x0F
;----------------------------------------

mov al, 0x00
out dx, al
mov al, 0x00
out dx, al

mov al, 0x01
out dx, al
mov al, 0x01
out dx, al

mov al, 0x02
out dx, al
mov al, 0x02
out dx, al

mov al, 0x03
out dx, al
mov al, 0x03
out dx, al

mov al, 0x04
out dx, al
mov al, 0x04
out dx, al

mov al, 0x05
out dx, al
mov al, 0x05
out dx, al

mov al, 0x06
out dx, al
mov al, 0x06
out dx, al

mov al, 0x07
out dx, al
mov al, 0x07
out dx, al

mov al, 0x08
out dx, al
mov al, 0x08
out dx, al

mov al, 0x09
out dx, al
mov al, 0x09
out dx, al

mov al, 0x0A
out dx, al
mov al, 0x0A
out dx, al

mov al, 0x0B
out dx, al
mov al, 0x0B
out dx, al

mov al, 0x0C
out dx, al
mov al, 0x0C
out dx, al

mov al, 0x0D
out dx, al
mov al, 0x0D
out dx, al

mov al, 0x0E
out dx, al
mov al, 0x0E
out dx, al

mov al, 0x0F
out dx, al
mov al, 0x0F
out dx, al

;----------------------------------------
; Attribute Mode Control Register (0x10)
;----------------------------------------
; 0x01 = text mode, blink enabled, 4-bit color

mov al, 0x10
out dx, al
mov al, 0x0C
out dx, al

;----------------------------------------
; Overscan Color Register (0x11)
;----------------------------------------
; border color = 0

mov al, 0x11
out dx, al
mov al, 0x00
out dx, al

;----------------------------------------
; Color Plane Enable Register (0x12)
;----------------------------------------
; enable planes 0–3

mov al, 0x12
out dx, al
mov al, 0x0F
out dx, al

;----------------------------------------
; Horizontal Pixel Panning (0x13)
;----------------------------------------
; no panning

mov al, 0x13
out dx, al
mov al, 0x00
out dx, al

;----------------------------------------
; Color Select Register (0x14)
;----------------------------------------
; upper DAC bits = 0

mov al, 0x14
out dx, al
mov al, 0x00
out dx, al

;----------------------------------------
; Attribute Controller Enable Display
;----------------------------------------

mov al, 0x20        ; PAS=1, index=0
out dx, al

