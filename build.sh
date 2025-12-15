#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 1. ブートローダーを bin に
nasm -f bin src/boot/boot.asm -o build/boot.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/kernel/switch32.asm -o build/switch32.o

# 3. カーネル C をオブジェクトファイルに
gcc -m32 -ffreestanding -I./src -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/rtc.c -o build/rtc.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -c src/arch/x86/a20.c -o build/a20.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/cmos.o \
  build/console.o \
  build/rtc.o \
  build/init.o \
  build/pic.o \
  build/a20.o \
  build/string.o

# 5. ELF → バイナリ
objcopy -O binary build/kernel.elf build/kernel.bin

# 16KBになるように0埋め(ブートローダを変更しなくて良いように大きさを固定する)
# まず大きさを確認
size=$(stat -c%s build/kernel.bin)
# 16 KB = 16384 バイト
pad=$((16384 - size))
# pad が正の値ならゼロを追記
if [ $pad -gt 0 ]; then
    dd if=/dev/zero bs=1 count=$pad >> build/kernel.bin
fi

# 6. ブート＋カーネルを結合してディスクイメージ作成
cat build/boot.bin build/kernel.bin > build/disk.img

# 7. QEMU で実行
qemu-system-i386 -hda build/disk.img

# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=33 conv=notrunc

