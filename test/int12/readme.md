
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
<img width="810" height="548" alt="截图 2026-10-24 17-04-03" src="https://github.com/user-attachments/assets/bddaf18a-4de6-41e9-a167-40a0bb3d02ee" />


V30復刻機 : AX=0x027F (639 KB​)
<img width="1920" height="1440" alt="bef576930bb678" src="https://github.com/user-attachments/assets/939a50e5-3d74-4ebf-aa88-8d23a88cc7db" />


東芝SS M10 : AX=0x0268 (616 KB​)
<img width="1440" height="1920" alt="efb389620f394" src="https://github.com/user-attachments/assets/6113228f-5d3f-4ad4-bc85-7d918d9449d9" />


# 自作BIOSでも実装したので検証

```
qemu-system-i386 -bios mybios.bin -hda test.bin
```
