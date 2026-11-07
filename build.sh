#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 起動ペンギン画像をHファイルへ変換
xxd -i src/kernel/penguin.bmp > src/kernel/penguin_bmp.h

# 自作BIOSのビルド
nasm -f bin src/bios/mybios.asm -o build/mybios.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/boot/switch32.asm -o build/switch32.o

# 3. C をオブジェクトファイルに
# -m32 : 32ビットの機械語を生成
# -ffreestanding : OS標準ライブラリを使わない。開始点がmain()でなくても良くなる
# -O2 : 最適化する
# -c : コンパイルのみ、リンクはしない　これがないと gcc は最終的な実行ファイル（.exe や a.out）を作ろうとする
# -machine isapcを使う場合、新しい機械語が生成される場合は-march=i486を指定する　Commit e4b87aa参照
gcc -m32 -ffreestanding -I./src -march=i486 -O2 -c src/kernel/kernel.c -o build/kernel.o

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/kernel/command.c -o build/command.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/kernel/command.c -o build/command.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/kernel/user_exec.c -o build/user_exec.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/kernel/user_exec.c -o build/user_exec.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/drivers/cmos.c -o build/cmos.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/console.c -o build/console.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/pic.c -o build/pic.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/idt.c -o build/idt.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/gdt.c -o build/gdt.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/gdt.c -o build/gdt.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/paging.c -o build/paging.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/paging.c -o build/paging.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/ata.c -o build/ata.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/drivers/ata.c -o build/ata.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/fs/fs_mount.c -o build/fs_mount.o

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/bootdev.c -o build/bootdev.o

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/atapi.c -o build/atapi.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/drivers/atapi.c -o build/atapi.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/drivers/keyboard.c -o build/keyboard.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/panic.c -o build/panic.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/panic.c -o build/panic.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/syscall.c -o build/syscall.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/syscall.c -o build/syscall.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/lib/string.c -o build/string.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/lib/string.c -o build/string.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/kernel/mem.c -o build/mem.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/kernel/mem.c -o build/mem.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/fs/fat16.c -o build/fat16.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/fs/fat16.c -o build/fat16.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/fs/fs_file.c -o build/fs_file.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/fs/fs_file.c -o build/fs_file.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/vbe.c -o build/vbe.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/vbe.c -o build/vbe.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/pit.c -o build/pit.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/pit.c -o build/pit.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/cpu.c -o build/cpu.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/cpu.c -o build/cpu.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/x86/timer.c -o build/timer.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/x86/timer.c -o build/timer.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/lib/itoa.c -o build/itoa.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/lib/itoa.c -o build/itoa.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/lib/bmp.c -o build/bmp.o # isapcのcmovs対策必須
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/lib/bmp.c -o build/bmp.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/debug/serial.c -o build/serial.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/debug/serial.c -o build/serial.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/debug/debug_print.c -o build/debug_print.o # -O2厳禁
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/debug/debug_print.c -o build/debug_print.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/debug/detect.c -o build/detect.o
gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-plt -masm=intel -fverbose-asm -S src/debug/detect.c -o build/detect.asm

gcc -m32 -ffreestanding -I./src -march=i486 -fno-pic -fno-pie -fno-asynchronous-unwind-tables -c src/drivers/ata_block.c -o build/ata_block.o

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
  build/ata_block.o \
  build/atapi.o \
  build/fs_mount.o \
  build/bootdev.o \
  build/panic.o \
  build/keyboard.o \
  build/string.o \
  build/console.o \
  build/mem.o \
  build/syscall.o \
  build/syscall_entry.o \
  build/user_exec.o \
  build/fat16.o \
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
  build/detect.o
  
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

# ---FAT16イメージ生成----------------------------------------------------------------
nasm -f bin src/boot/vbr.asm -o build/vbr.bin

dd if=/dev/zero of=build/fat16.img bs=1M count=16
mkfs.fat -F16 -s 8 -S 512 build/fat16.img
dd if=build/fat16.img of=build/vbr_until_start.bin bs=1 count=62
dd if=build/vbr_until_start.bin of=build/vbr.bin bs=1 seek=0 conv=notrunc
dd if=build/vbr.bin of=build/fat16.img bs=512 count=1 conv=notrunc

echo 'drive x: file="build/fat16.img" fat_bits=16' > mtoolsrc
export MTOOLSRC=$PWD/mtoolsrc

mcopy build/kernel.bin x:KERNEL.BIN

mcopy build/test2.bin   x:TEST2.BIN
mcopy build/test3.bin   x:TEST3.BIN
mcopy build/cat.bin     x:CAT.BIN
mcopy build/memtest.bin x:MEMTEST.BIN

cp user/test.txt build/test.txt
mcopy build/test.txt x:TEST.TXT

mmd x:SUB

# qemu-system-i386 -hda build/fat16.img  -monitor stdio

qemu-system-i386   -bios build/mybios.bin   -drive file=build/fat16.img,format=raw,if=ide,index=0   -serial stdio   -machine isapc   -vga std

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

# qemu-system-i386 -m 64M  -enable-kvm -cpu qemu32,+pse -device ide-cd,bus=ide.0,unit=0 -cdrom build/os.iso -boot d -serial stdio

# -serial stdio
