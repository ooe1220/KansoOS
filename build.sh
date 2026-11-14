#!/bin/bash
mkdir -p build # githubにbuildフォルダをあげていない為ここで追加。
clear

# 逆アセンブル結果格納用フォルダ生成
while IFS= read -r d; do
  mkdir -p "build/disasm/${d#src/}"
done < <(find src -type d)

while IFS= read -r d; do
  mkdir -p "build/disasm/user/${d#user/}"
done < <(find user -type d)

# 起動ペンギン画像をHファイルへ変換
xxd -i src/kernel/penguin.bmp > src/kernel/penguin_bmp.h

# 自作BIOSのビルド
nasm -f bin src/bios/mybios.asm -o build/mybios.bin

# 2. 追加アセンブリをオブジェクトに
nasm -f elf32 src/boot/switch32.asm -o build/switch32.o

# 3. C をオブジェクトファイルに
# -m32 : 32ビットの機械語を生成
# -ffreestanding : OS標準ライブラリを使わない。開始点がmain()でなくても良くなる
# -c : コンパイルのみ、リンクはしない　これがないと gcc は最終的な実行ファイル（.exe や a.out）を作ろうとする
# -machine isapcを使う場合、新しい機械語が生成される場合は-march=i486を指定する　Commit e4b87aa参照

# !!!!!!!!!!!!!!!!!!!O2にするとQEMUでUSBからの起動に失敗する。どれが原因かわからない為段階的にO2へ変えていく

CC="gcc -m32 -ffreestanding -I./src -march=i486"
CF="$CC -fno-pic -fno-pie -fno-asynchronous-unwind-tables"
ASM="$CF -fno-plt -masm=intel -fverbose-asm -S"

$CF   -O2 -c src/kernel/kernel.c -o build/kernel.o
$ASM  -O2 src/kernel/kernel.c -o build/disasm/kernel/kernel.asm

$CF   -O2 -c src/kernel/command.c -o build/command.o
$ASM  -O0 src/kernel/command.c -o build/disasm/kernel/command_O0.asm
$ASM  -O2 src/kernel/command.c -o build/disasm/kernel/command_O2.asm

$CF   -O2 -c src/kernel/shell.c -o build/shell.o
$ASM  -O0 src/kernel/shell.c -o build/disasm/kernel/shell.asm
$ASM  -O2 src/kernel/shell.c -o build/disasm/kernel/shell.asm

$CF   -O2 -c src/kernel/user_exec.c -o build/user_exec.o
$ASM  -O0 src/kernel/user_exec.c -o build/disasm/kernel/user_exec_O0.asm
$ASM  -O2 src/kernel/user_exec.c -o build/disasm/kernel/user_exec_O2.asm

$CF   -O0 -c src/kernel/user_exec_asm.S -o build/user_exec_asm.o

$CF   -O2 -c src/drivers/cmos.c -o build/cmos.o
$ASM  -O0 src/drivers/cmos.c -o build/disasm/drivers/cmos_O0.asm
$ASM  -O2 src/drivers/cmos.c -o build/disasm/drivers/cmos_O2.asm

$CF   -O2 -c src/x86/console.c -o build/console.o
$ASM  -O0 src/x86/console.c -o build/disasm/x86/console_O0.asm
$ASM  -O2 src/x86/console.c -o build/disasm/x86/console_O2.asm

$CF   -O2 -c src/x86/pic.c -o build/pic.o
$ASM  -O0 src/x86/pic.c -o build/disasm/x86/pic_O0.asm
$ASM  -O2 src/x86/pic.c -o build/disasm/x86/pic_O2.asm

$CF   -O0 -c src/x86/idt.c -o build/idt.o
$ASM  -O0 src/x86/idt.c -o build/disasm/x86/idt_O0.asm
$ASM  -O2 src/x86/idt.c -o build/disasm/x86/idt_O2.asm

$CF   -O0 -c src/drivers/ata.c -o build/ata.o
$ASM  -O0 src/drivers/ata.c -o build/disasm/drivers/ata_O0.asm
$ASM  -O2 src/drivers/ata.c -o build/disasm/drivers/ata_O2.asm

$CF   -O0 -c src/fs/fs_mount.c -o build/fs_mount.o
$ASM  -O0 src/fs/fs_mount.c -o build/disasm/fs/fs_mount_O0.asm
$ASM  -O2 src/fs/fs_mount.c -o build/disasm/fs/fs_mount_O2.asm

$CF   -O0 -c src/drivers/bootdev.c -o build/bootdev.o
$ASM  -O0 src/drivers/bootdev.c -o build/disasm/drivers/bootdev_O0.asm
$ASM  -O2 src/drivers/bootdev.c -o build/disasm/drivers/bootdev_O2.asm

$CF   -O0 -c src/drivers/atapi.c -o build/atapi.o
$ASM  -O0 src/drivers/atapi.c -o build/disasm/drivers/atapi_O0.asm
$ASM  -O2 src/drivers/atapi.c -o build/disasm/drivers/atapi_O2.asm

$CF   -O0 -c src/drivers/pci.c -o build/pci.o
$ASM  -O0 src/drivers/pci.c -o build/disasm/drivers/pci_O0.asm
$ASM  -O2 src/drivers/pci.c -o build/disasm/drivers/pci_O2.asm

$CF   -O0 -c src/drivers/keyboard.c -o build/keyboard.o
$ASM  -O0 src/drivers/keyboard.c -o build/disasm/drivers/keyboard_O0.asm
$ASM  -O2 src/drivers/keyboard.c -o build/disasm/drivers/keyboard_O2.asm

$CF   -O0 -c src/x86/panic.c -o build/panic.o
$ASM  -O0 src/x86/panic.c -o build/disasm/x86/panic_O0.asm
$ASM  -O2 src/x86/panic.c -o build/disasm/x86/panic_O2.asm

$CF   -O0 -c src/x86/syscall.c -o build/syscall.o
$ASM  -O0 src/x86/syscall.c -o build/disasm/x86/syscall_O0.asm
$ASM  -O2 src/x86/syscall.c -o build/disasm/x86/syscall_O2.asm

$CF   -O0 -c src/lib/string.c -o build/string.o
$ASM  -O0 src/lib/string.c -o build/disasm/lib/string_O0.asm
$ASM  -O2 src/lib/string.c -o build/disasm/lib/string_O2.asm

$CF   -O0 -c src/kernel/mem.c -o build/mem.o
$ASM  -O0 src/kernel/mem.c -o build/disasm/kernel/mem_O0.asm
$ASM  -O2 src/kernel/mem.c -o build/disasm/kernel/mem_O2.asm

$CF   -O0 -c src/fs/fat16.c -o build/fat16.o
$ASM  -O0 src/fs/fat16.c -o build/disasm/fs/fat16_O0.asm
$ASM  -O2 src/fs/fat16.c -o build/disasm/fs/fat16_O2.asm

$CF   -O0 -c src/fs/iso9660.c -o build/iso9660.o
$ASM  -O0 src/fs/iso9660.c -o build/disasm/fs/iso9660_O0.asm
$ASM  -O2 src/fs/iso9660.c -o build/disasm/fs/iso9660_O2.asm

$CF   -O0 -c src/fs/fs_file.c -o build/fs_file.o
$ASM  -O0 src/fs/fs_file.c -o build/disasm/fs/fs_file_O0.asm
$ASM  -O2 src/fs/fs_file.c -o build/disasm/fs/fs_file_O2.asm

$CF   -O0 -c src/x86/vbe.c -o build/vbe.o
$ASM  -O0 src/x86/vbe.c -o build/disasm/x86/vbe_O0.asm
$ASM  -O2 src/x86/vbe.c -o build/disasm/x86/vbe_O2.asm

$CF   -O0 -c src/x86/gfx.c -o build/gfx.o
$ASM  -O0 src/x86/gfx.c -o build/disasm/x86/gfx_O0.asm
$ASM  -O2 src/x86/gfx.c -o build/disasm/x86/gfx_O2.asm

$CF   -O0 -c src/x86/pit.c -o build/pit.o
$ASM  -O0 src/x86/pit.c -o build/disasm/x86/pit_O0.asm
$ASM  -O2 src/x86/pit.c -o build/disasm/x86/pit_O2.asm

$CF   -O0 -c src/x86/cpu.c -o build/cpu.o
$ASM  -O0 src/x86/cpu.c -o build/disasm/x86/cpu_O0.asm
$ASM  -O2 src/x86/cpu.c -o build/disasm/x86/cpu_O2.asm

$CF   -O0 -c src/x86/timer.c -o build/timer.o
$ASM  -O0 src/x86/timer.c -o build/disasm/x86/timer_O0.asm
$ASM  -O2 src/x86/timer.c -o build/disasm/x86/timer_O2.asm

$CF   -O0 -c src/lib/itoa.c -o build/itoa.o
$ASM  -O0 src/lib/itoa.c -o build/disasm/lib/itoa_O0.asm
$ASM  -O2 src/lib/itoa.c -o build/disasm/lib/itoa_O2.asm

$CF   -O0 -c src/lib/bmp.c -o build/bmp.o   # isapcのcmovs対策必須
$ASM  -O0 src/lib/bmp.c -o build/disasm/lib/bmp_O0.asm
$ASM  -O2 src/lib/bmp.c -o build/disasm/lib/bmp_O2.asm

$CF   -O0 -c src/debug/serial.c -o build/serial.o
$ASM  -O0 src/debug/serial.c -o build/disasm/debug/serial_O0.asm
$ASM  -O2 src/debug/serial.c -o build/disasm/debug/serial_O2.asm

$CF   -O0 -c src/debug/debug_print.c -o build/debug_print.o   # -O2厳禁
$ASM  -O0 src/debug/debug_print.c -o build/disasm/debug/debug_print.asm

$CF   -O0 -c src/debug/detect.c -o build/detect.o
$ASM  -O0 src/debug/detect.c -o build/disasm/debug/detect_O0.asm
$ASM  -O2 src/debug/detect.c -o build/disasm/debug/detect_O2.asm

$CF   -O0 -c src/drivers/ata_block.c -o build/ata_block.o
$ASM  -O0 src/drivers/ata_block.c -o build/disasm/drivers/ata_block_O0.asm
$ASM  -O2 src/drivers/ata_block.c -o build/disasm/drivers/ata_block_O2.asm

$CF   -O0 -c src/drivers/atapi_block.c -o build/atapi_block.o
$ASM  -O0 src/drivers/ata_block.c -o build/disasm/drivers/atapi_block_O0.asm
$ASM  -O2 src/drivers/ata_block.c -o build/disasm/drivers/atapi_block_O2.asm

# フラグの並びが違う単発はそのまま
nasm -f elf32 src/boot/switch32.asm -o build/switch32.o
gcc -m32 -ffreestanding -I./src -c src/x86/font_8x16.c -o build/font_8x16.o
gcc -m32 -ffreestanding -fno-pic -fno-pie -c src/x86/syscall_entry.S -o build/syscall_entry.o

gcc -m32 -ffreestanding -I./src -c src/x86/font_8x16.c -o build/font_8x16.o
gcc -m32 -ffreestanding -fno-pic -fno-pie -c src/x86/syscall_entry.S -o build/syscall_entry.o

# 4. リンカで ELF 作成
ld -m elf_i386 -T src/linker.ld -o build/kernel.elf \
  build/switch32.o \
  build/kernel.o \
  build/command.o \
  build/shell.o \
  build/cmos.o \
  build/pic.o \
  build/idt.o \
  build/ata.o \
  build/ata_block.o \
  build/atapi.o \
  build/pci.o \
  build/atapi_block.o \
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
  build/user_exec_asm.o \
  build/fat16.o \
  build/iso9660.o \
  build/fs_file.o \
  build/cpu.o \
  build/vbe.o \
  build/gfx.o \
  build/font_8x16.o \
  build/pit.o \
  build/timer.o \
  build/serial.o \
  build/itoa.o \
  build/bmp.o \
  build/debug_print.o \
  build/detect.o

# 全体的に逆アセンブル
mkdir -p build/disasm/
#objdump -d -M intel -S build/kernel.elf | c++filt > build/disasm/disasm_kernel.txt
objdump -D -M intel -S build/kernel.elf | c++filt > build/disasm/disasm_kernel.txt  

readelf -S build/kernel.elf > build/disasm/section.txt

# 5. kernel.elf(ELF) → kernel.bin(バイナリ)
objcopy -O binary build/kernel.elf build/kernel.bin

# kernel.binを64KBまでへ0埋め
current=$(stat -c%s build/kernel.bin)
dd if=/dev/zero bs=1 count=$((65536 - current)) >> build/kernel.bin

# 6. ユーザー空間
gcc -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/start.S -o build/start.o

## test2.c
gcc -O2 -ffreestanding -march=i486  -nostdlib -fno-pic -fno-pie -m32 -c user/test2.c -o build/test2.o
ld -m elf_i386 -T user/linker.ld build/start.o build/test2.o -o build/test2.elf
objcopy -O binary build/test2.elf build/test2.bin

## test3.c
gcc -O2 -ffreestanding -march=i486  -nostdlib -fno-pic -fno-pie -m32 -c user/test3.c -o build/test3.o
ld -m elf_i386 -T user/linker.ld build/start.o build/test3.o -o build/test3.elf
objcopy -O binary build/test3.elf build/test3.bin

## cat.c
gcc -O2 -ffreestanding -march=i486  -nostdlib -fno-pic -fno-pie -m32 -c user/cat.c -o build/cat.o
ld -m elf_i386 -T user/linker.ld build/start.o build/cat.o -o build/cat.elf
objcopy -O binary build/cat.elf build/cat.bin

## MEMTEST.BIN
gcc -O2 -ffreestanding -nostdlib -fno-pic -fno-pie -m32 -c user/memtest.c -o build/memtest.o
ld -m elf_i386 -T user/linker.ld build/start.o build/memtest.o -o build/memtest.elf
objcopy -O binary build/memtest.elf build/memtest.bin

## GUITEST.BIN
gcc -O2 -ffreestanding -march=i486  -nostdlib -fno-pic -fno-pie -m32 -c user/guitest.c -o build/guitest.o
ld -m elf_i386 -T user/linker.ld build/start.o build/guitest.o -o build/guitest.elf
objcopy -O binary build/guitest.elf build/guitest.bin

## GETCHAR
gcc -O2 -ffreestanding -march=i486  -nostdlib -fno-pic -fno-pie -m32 -c user/getchar.c -o build/getchar.o
ld -m elf_i386 -T user/linker.ld build/start.o build/getchar.o -o build/getchar.elf
objcopy -O binary build/getchar.elf build/getchar.bin

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
mcopy build/guitest.bin x:GUITEST.BIN
mcopy build/getchar.bin x:GETCHAR.BIN

cp user/test.txt build/test.txt
mcopy build/test.txt x:TEST.TXT

mmd x:SUB

# qemu-system-i386 -hda build/fat16.img  -monitor stdio
# qemu-system-i386   -bios build/mybios.bin   -drive file=build/fat16.img,format=raw,if=ide,index=0   -serial stdio   -machine isapc   -vga std

# ---CD起動用のISO生成----------------------------------------------------------------

nasm -f bin src/boot/cdboot.asm -o build/cdboot.bin

mkdir -p build/iso_root/boot
cp build/cdboot.bin build/iso_root/boot/
cp build/kernel.bin  build/iso_root/
cp build/cat.bin  build/iso_root/
cp build/memtest.bin  build/iso_root/
cp build/guitest.bin  build/iso_root/
cp build/getchar.bin  build/iso_root/
cp build/test.txt  build/iso_root/

 xorriso -as mkisofs \
  -o build/os.iso \
  -b boot/cdboot.bin \
  -no-emul-boot \
  -boot-load-size 4 \
  build/iso_root

# qemu-system-i386 -m 64M  -enable-kvm -cpu qemu32,+pse -device ide-cd,bus=ide.0,unit=0 -cdrom build/os.iso -boot d -serial stdio
# -serial stdio

# kernel.cのLBAを探す ファイル増えたら変わる　今後はブートローダ内で探すように改良予定
# isoinfo -l -i build/os.iso | grep KERNEL
# dd if=build/os.iso bs=2048 skip=37 count=32 | cmp - build/kernel.bin && echo OK
