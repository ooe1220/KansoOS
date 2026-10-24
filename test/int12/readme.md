
# int 12h

1MB 以下の通常メモリが何 KBあるかをAXレジスタに返す。
1MB以上は取れない

# 実行

```bash
nasm -f bin test.asm -o test.bin
qemu-system-i386 -hda test.bin

lsblk
sudo dd if=test.bin of=/dev/sdb bs=512 count=1 conv=notrunc
```

# 実行結果

QEMU : AX=0x027F (639 KB​)

V30復刻機 : AX=0x027F (639 KB​)

東芝SS M10 : AX=0x0268 (616 KB​)


# 自作BIOSでも実装したので検証

```
qemu-system-i386 -bios mybios.bin -hda test.bin
```
