#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 1. ブートローダーを bin に
nasm -f bin src/boot/boot.asm -o build/boot.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/kernel/switch32.asm -o build/switch32.o
nasm -f elf32 src/arch/x86/isr.asm -o build/isr.o

# 3. カーネル C をオブジェクトファイルに
gcc -m32 -ffreestanding -I./src -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/rtc.c -o build/rtc.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/a20.c -o build/a20.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -c src/lib/string.c -o build/string.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/cmos.o \
  build/console.o \
  build/rtc.o \
  build/pic.o \
  build/a20.o \
  build/idt.o \
  build/isr.o \
  build/string.o

# 5. ELF → バイナリ
objcopy -O binary build/kernel.elf build/kernel.bin

# 仮想HDD作成
dd if=/dev/zero of=build/disk.img bs=1M count=1
# boot.bin を先頭セクタに書き込む（1セクタ = 512B）
dd if=build/boot.bin of=build/disk.img bs=512 count=1 conv=notrunc

# kernel.bin をその次のセクタから書き込む
dd if=build/kernel.bin of=build/disk.img bs=512 seek=1 conv=notrunc

# 7. QEMU で実行
qemu-system-i386 -hda build/disk.img

# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=33 conv=notrunc

