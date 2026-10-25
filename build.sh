#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 起動ペンギン画像をHファイルへ変換
xxd -i src/kernel/penguin.bmp > src/kernel/penguin_bmp.h

# 自作BIOSのビルド
nasm -f bin src/bios/mybios.asm -o build/mybios.bin

# 1. ブートローダーを bin に
nasm -f bin src/boot/mbr.asm -o build/mbr.bin
nasm -f bin src/boot/vbr.asm -o build/vbr.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/kernel/switch32.asm -o build/switch32.o

# 3. C をオブジェクトファイルに
# -m32 : 32ビットの機械語を生成
# -ffreestanding : OS標準ライブラリを使わない。開始点がmain()でなくても良くなる
# -O2 : 最適化する
# -c : コンパイルのみ、リンクはしない　これがないと gcc は最終的な実行ファイル（.exe や a.out）を作ろうとする
# -machine isapcを使う場合、新しい機械語が生成される場合は-march=i486を指定する　Commit e4b87aa参照
gcc -m32 -ffreestanding -I./src -march=i486 -O2 -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/kernel/command.c -o build/command.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/kernel/user_exec.c -o build/user_exec.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/drivers/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/gdt.c -o build/gdt.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/paging.c -o build/paging.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/drivers/ata.c -o build/ata.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/drivers/atapi.c -o build/atapi.o
gcc -m32 -ffreestanding -I./src -march=i486 -O2 -c src/drivers/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/panic.c -o build/panic.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/syscall.c -o build/syscall.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/lib/string.c -o build/string.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/kernel/mem.c -o build/mem.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/fs/dir.c -o build/dir.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/fs/fat16_file.c -o build/fat16_file.o
gcc -m32 -ffreestanding -I./src -march=i486  -O2 -c src/fs/fs_file.c -o build/fs_file.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/vbe.c -o build/vbe.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/pit.c -o build/pit.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/cpu.c -o build/cpu.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/x86/timer.c -o build/timer.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/lib/itoa.c -o build/itoa.o
gcc -m32 -ffreestanding -I./src -march=i486 -c src/lib/bmp.c -o build/bmp.o # isapcのcmovs対策
gcc -m32 -ffreestanding -I./src -march=i486  -c src/debug/serial.c -o build/serial.o
gcc -m32 -ffreestanding -I./src -march=i486  -c src/debug/debug_print.c -o build/debug_print.o # -O2厳禁
gcc -m32 -ffreestanding -I./src -march=i486  -c src/debug/detect.c -o build/detect.o

gcc -m32 -ffreestanding -I./src -c src/x86/font_8x16.c -o build/font_8x16.o
gcc -m32 -ffreestanding -fno-pic -fno-pie -c src/x86/syscall_entry.S -o build/syscall_entry.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/command.o \
  build/cmos.o \
  build/pic.o \
  build/paging.o \
  build/idt.o \
  build/gdt.o \
  build/ata.o \
  build/atapi.o \
  build/panic.o \
  build/keyboard.o \
  build/string.o \
  build/console.o \
  build/mem.o \
  build/syscall.o \
  build/syscall_entry.o \
  build/user_exec.o \
  build/fat16_file.o \
  build/fs_file.o \
  build/cpu.o \
  build/vbe.o \
  build/font_8x16.o \
  build/pit.o \
  build/timer.o \
  build/serial.o \
  build/itoa.o \
  build/bmp.o \
  build/debug_print.o \
  build/detect.o \
  build/dir.o
  
# 5. kernel.elf(ELF) → kernel.bin(バイナリ)
objcopy -O binary build/kernel.elf build/kernel.bin

# kernel.binを64KBまでへ0埋め
current=$(stat -c%s build/kernel.bin)
dd if=/dev/zero bs=1 count=$((65536 - current)) >> build/kernel.bin

# 6. ユーザー空間
gcc -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/start.S -o build/start.o

## test2.c
gcc -O2 -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/test2.c -o build/test2.o
ld -m elf_i386 -T user/linker.ld build/start.o build/test2.o -o build/test2.elf
objcopy -O binary build/test2.elf build/test2.bin

## test3.c
gcc -O2 -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/test3.c -o build/test3.o
ld -m elf_i386 -T user/linker.ld build/start.o build/test3.o -o build/test3.elf
objcopy -O binary build/test3.elf build/test3.bin

## cat.c
gcc -O2 -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/cat.c -o build/cat.o
ld -m elf_i386 -T user/linker.ld build/start.o build/cat.o -o build/cat.elf
objcopy -O binary build/cat.elf build/cat.bin

## MEMTEST.BIN
gcc -O2 -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/memtest.c -o build/memtest.o
ld -m elf_i386 -T user/linker.ld build/start.o build/memtest.o -o build/memtest.elf
objcopy -O binary build/memtest.elf build/memtest.bin

## test.txt
cp user/test.txt build/test.txt

## 成果物をdisk.imgに纏める
gcc tool/mkfs.c -o build/mkfs
build/mkfs


# ---CD起動用のISO生成----------------------------------------------------------------

nasm -f bin src/boot/cdboot.asm -o build/cdboot.bin

mkdir -p build/iso_root/boot
cp build/cdboot.bin build/iso_root/boot/
cp build/kernel.bin  build/iso_root/

 xorriso -as mkisofs \
  -o build/os.iso \
  -b boot/cdboot.bin \
  -no-emul-boot \
  -boot-load-size 4 \
  build/iso_root


  qemu-system-i386 -m 64M  -enable-kvm -cpu qemu32,+pse -device ide-cd,bus=ide.0,unit=0 -cdrom build/os.iso -boot d -serial stdio

# ------------------------------------------------------------------------------------------

# 7. QEMU で実行
# 標準BIOSで立ち上げる
# qemu-system-i386 -hda build/disk.img -monitor stdio
# qemu-system-i386 -hda build/disk.img -serial stdio

#qemu-system-i386 -hda build/disk.img -serial stdio -d in_asm,int,cpu_reset -D /tmp/qemu.log

# 自作BIOSで立ち上げる  isapcを使いVBEのVRAM起点を固定する
#  qemu-system-i386   -bios build/mybios.bin   -drive file=build/disk.img,format=raw,if=ide,index=0   -serial stdio   -machine isapc   -vga std
#  qemu-system-i386   -bios build/mybios.bin   -drive file=build/disk.img,format=raw,if=ide,index=0   -monitor stdio   -machine isapc   -vga std

#  qemu-system-i386   -bios build/mybios.bin   -drive file=build/disk.img,format=raw,if=ide,index=0   -serial stdio -vga std

#qemu-system-i386 -bios build/mybios.bin -drive file=build/disk.img,format=raw,if=ide,index=0 -serial stdio -machine isapc -vga std -d int -no-reboot -no-shutdown
#objdump -d build/kernel.elf | grep -B5 -A5 "b5d0"
  
# xp /32768hx 0x0
# xp /16384hx 0x10000
# info registers


# git status
# git diff

