BITS 16
org 0

; =========================
; FAT #1
; =========================
fat1:
    dw 0xFFF8        ; cluster 0
    dw 0xFFFF        ; cluster 1
    times (512 - 4) db 0


; =========================
; FAT #2
; =========================
fat2:
    dw 0xFFF8
    dw 0xFFFF
    times (512 - 4) db 0


; =========================
; Root Directory (32 entries)
; =========================
rootdir:

; ---- KERNEL.BIN ----
db 'KERNEL  BIN'     ; 8.3 name
db 0x20              ; ATTR = archive
db 0                 ; NT
db 0                 ; create time fine
dw 0                 ; create time
dw 0                 ; create date
dw 0                 ; access date
dw 0                 ; high cluster (FAT16)
dw 0                 ; write time
dw 0                 ; write date
dw 2                 ; start cluster (仮)
dd 0                 ; file size (未使用)

db 'TEST2   BIN'
db 0x20
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 213 ; 1814セクタ目
dd 0

db 'TEST3   BIN'
db 0x20
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 214 ; 1822セクタ目
dd 0

db 'TEST4   BIN'
db 0x20
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 215 ; 1830セクタ目
dd 0

; ---- TEST.TXT ----
db 'TEST    TXT'
db 0x20
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 216 ; 1838セクタ目
dd 0

; ---- 残り 項目 ----
times (32*32 - 64) db 0

