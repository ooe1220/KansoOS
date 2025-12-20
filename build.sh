#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 1. ブートローダーを bin に
nasm -f bin src/boot/mbr.asm -o build/mbr.bin
nasm -f bin src/boot/vbr.asm -o build/vbr.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/kernel/switch32.asm -o build/switch32.o
nasm -f elf32 src/arch/x86/isr.asm -o build/isr.o
nasm -f elf32 src/arch/x86/irq1.asm -o build/irq1.o

# 3. カーネル C をオブジェクトファイルに
gcc -m32 -ffreestanding -I./src -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/ata.c -o build/ata.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -I./src -c src/lib/string.c -o build/string.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/cmos.o \
  build/console.o \
  build/pic.o \
  build/idt.o \
  build/isr.o \
  build/ata.o \
  build/keyboard.o \
  build/irq1.o \
  build/string.o

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


# 7. QEMU で実行
qemu-system-i386 -hda build/disk.img
# qemu-system-i386 -hda build/disk.img -monitor stdio

#  xp /512bx 0x9000 # 読み込まれているか確認

# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=33 conv=notrunc

