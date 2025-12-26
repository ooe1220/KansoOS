#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 1. ブートローダーを bin に
nasm -f bin src/boot/mbr.asm -o build/mbr.bin
nasm -f bin src/boot/vbr.asm -o build/vbr.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/kernel/switch32.asm -o build/switch32.o

# 3. カーネル C をオブジェクトファイルに
gcc -m32 -ffreestanding -I./src -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -c src/kernel/command.c -o build/command.o
gcc -m32 -ffreestanding -I./src -c src/x86/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -c src/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -c src/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -c src/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -c src/x86/ata.c -o build/ata.o
gcc -m32 -ffreestanding -I./src -c src/x86/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -I./src -c src/x86/panic.c -o build/panic.o
gcc -m32 -ffreestanding -I./src -c src/lib/string.c -o build/string.o
gcc -m32 -ffreestanding -I./src -c src/fs/dir.c -o build/dir.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/command.o \
  build/cmos.o \
  build/console.o \
  build/pic.o \
  build/idt.o \
  build/ata.o \
  build/panic.o \
  build/keyboard.o \
  build/string.o \
  build/dir.o

# 5. ELF → バイナリ
objcopy -O binary build/kernel.elf build/kernel.bin

# 6. 仮想HDD作成
dd if=/dev/zero of=build/disk.img bs=1M count=1

# MBR を LBA 0 に書く (C=0, H=0, S=1)
dd if=build/mbr.bin of=build/disk.img bs=512 count=1 seek=0 conv=notrunc

# VBR を LBA 63 に書く (C=0, H=1, S=1)
dd if=build/vbr.bin of=build/disk.img bs=512 count=1 seek=63 conv=notrunc

# kernel.binを LBA 126 に書く (C=0, H=2, S=1)
dd if=build/kernel.bin of=build/disk.img bs=512 seek=126 conv=notrunc

## FAT16形式で初期化
nasm -f bin src/fs/disk_ini.asm -o build/disk_ini.bin
dd if=build/disk_ini.bin of=build/disk.img bs=512 seek=64 conv=notrunc


## ユーザー空間
gcc -m32 -ffreestanding -I./src -c user/test.c -o build/test.bin
dd if=build/test.bin of=build/disk.img bs=512 seek=1800 conv=notrunc ## カーネルと被らないように後ろへ置く

# 7. QEMU で実行
# qemu-system-i386 -hda build/disk.img
qemu-system-i386 -hda build/disk.img -monitor stdio

#  xp /512bx 0x10000 # 読み込まれているか確認

# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=10000 conv=notrunc

