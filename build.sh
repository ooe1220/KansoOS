#!/bin/bash
mkdir build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 自作BIOSのビルド
nasm -f bin bios/mybios.asm -o build/mybios.bin

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
gcc -m32 -ffreestanding -I./src -O2 -c src/kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -I./src -O2 -c src/kernel/command.c -o build/command.o
gcc -m32 -ffreestanding -I./src -c src/kernel/user_exec.c -o build/user_exec.o
gcc -m32 -ffreestanding -I./src -c src/kernel/debug.c -o build/debug.o
gcc -m32 -ffreestanding -I./src -O2 -c src/x86/cmos.c -o build/cmos.o
gcc -m32 -ffreestanding -I./src -O2 -c src/x86/console.c -o build/console.o
gcc -m32 -ffreestanding -I./src -O2 -c src/x86/pic.c -o build/pic.o
gcc -m32 -ffreestanding -I./src -c src/x86/idt.c -o build/idt.o
gcc -m32 -ffreestanding -I./src -O2 -c src/x86/ata.c -o build/ata.o
gcc -m32 -ffreestanding -I./src -O2 -c src/x86/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -I./src -c src/x86/panic.c -o build/panic.o
gcc -m32 -ffreestanding -I./src -c src/x86/syscall.c -o build/syscall.o
gcc -m32 -ffreestanding -I./src -c src/lib/string.c -o build/string.o
gcc -m32 -ffreestanding -I./src -O2 -c src/fs/dir.c -o build/dir.o
gcc -m32 -ffreestanding -I./src -c src/mem/heap.c -o build/heap.o
gcc -m32 -ffreestanding -I./src -c src/mem/malloc.c -o build/malloc.o
gcc -m32 -ffreestanding -I./src -c src/mem/memory_utils.c -o build/memory_utils.o
gcc -m32 -ffreestanding -I./src -c src/mem/calloc_realloc.c -o build/calloc_realloc.o
gcc -m32 -ffreestanding -I./src -c src/mem/free.c -o build/free.o

gcc -m32 -ffreestanding -fno-pic -fno-pie -c src/x86/syscall_entry.S -o build/syscall_entry.o

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


#ユーザー空間
gcc -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/test2.c -o build/test2.o
gcc -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/start.S -o build/start.o

ld -m elf_i386 \
   -T user/linker.ld \
   build/start.o \
   build/test2.o \
   -o build/test2.elf

objcopy -O binary build/test2.elf build/test2.bin
## objdump -D -b binary -m i386 build/test2.bin
dd if=build/test2.bin of=build/disk.img bs=512 seek=1814 conv=notrunc

# 7. QEMU で実行
# 標準BIOSで立ち上げる
# qemu-system-i386 -hda build/disk.img -monitor stdio

# 自作BIOSで立ち上げる
   qemu-system-i386 \
   -bios build/mybios.bin\
   -drive file=build/disk.img,format=raw,if=ide,index=0 \
   -monitor stdio
   #-serial stdio



# USBメモリへ書き込む　sdXはlsblkの結果を参照する
# lsblk
# sudo dd if=build/disk.img of=/dev/sdb bs=512 count=10000 conv=notrunc


## レジスタ退避方法
# ```
# saved_ax dw 0
# mov [saved_ax], ax
# mov dx,[saved_dx]
# ```

