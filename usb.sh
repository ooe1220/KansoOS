#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

gcc -m32 -ffreestanding -I./src -c usb/kernel.c -o build/kernel.o

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
  build/syscall.o \
  build/syscall_entry.o \
  build/user_exec.o \
  build/heap.o \
  build/malloc.o \
  build/memory_utils.o \
  build/calloc_realloc.o \
  build/free.o \
  build/debug.o \
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

# 7. QEMU で実行
# qemu-system-i386 -hda build/disk.img
qemu-system-i386 -hda build/disk.img -monitor stdio

#  xp /512bx 0x10000 # 読み込まれているか確認

# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=10000 conv=notrunc

