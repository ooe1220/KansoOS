BIOS開発ではprintf()やint10hが使用出来ない為、以下の方法でデバッグを行います。

# -serial stdio

## 到達確認
```
   mov dx, 0x3F8
   mov al, '-'
   out dx, al
```

# -monitor stdio

## レジスタ値確認可能
`hlt`で止めて、`(qemu)info registers`実行

`xp /512bx 0x7c00` メモリ上に読み込まれているか確認

xp /512bx 0x8000
xp /3000bx 0x8000

## QEMU字体

字体ひゃQEMUソースコード中のものを拝借しています。
`qemu-11.1.0/roms/openbios/libopenbios/font_8x16.c`
`/qemu-11.1.0/roms/openbios/include/libopenbiosc/fontdata.h`

以下を実行して字体バイナリを生成しておく
```bash
gcc -c bios/font_8x16.c -o bios/font_8x16.o
objcopy -O binary bios/font_8x16.o bios/font_8x16.bin
```

