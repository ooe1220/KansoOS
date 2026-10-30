
# 原理

8086/8088と80286以降のCPUで`push sp`を実行した時にスタックに積まれる値の差異を利用する。

https://www.pcjs.org/documents/manuals/intel/80286/real_mode/

> The PUSH SP instruction pushes a different value on the iAPX 286 than on the iAPX 86,88,186. The value pushed onto the stack by the 80286 is the value of SP before the push instruction executes. The value pushed onto the stack by the 8086/88/186 is the SP value after the push instruction executes.

# 実行

```bash
nasm -f bin test.asm -o test.bin
qemu-system-i386 -hda test.bin

lsblk
sudo dd if=test.bin of=/dev/sdb bs=512 count=1 conv=notrunc
```

# 実行結果

QEMU
<img width="810" height="548" alt="截图 2026-10-30 19-59-44" src="https://github.com/user-attachments/assets/9b0ee32f-071d-4eb2-8109-27bd2356dc36" />


東芝SS M10 及び V30復刻機
<img width="1440" height="1920" alt="85306033ce9f88" src="https://github.com/user-attachments/assets/6984c609-8320-4d0e-99e1-3c02036f81b6" />


