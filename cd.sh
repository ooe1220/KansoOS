#!/bin/bash

# QEMUでは動くが実機では動かない
 # 64kbまで0埋め
# 現在の大いさ取得
current=$(stat -c%s build/kernel.bin)

# 足りない分を末尾に追加
dd if=/dev/zero bs=1 count=$((65536 - current)) >> build/kernel.bin

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
  
    # isoinfo -l -i build/os.iso

  qemu-system-i386 -cdrom build/os.iso -boot d -monitor stdio
  # (qemu)xp /100xb 0x8000
  
qemu-system-i386 -cdrom build/os.iso -boot d -serial stdio
  
  
  # DVD
  sudo dvd+rw-format -blank=fast /dev/sr0
  sudo growisofs -dvd-compat -Z /dev/sr0=build/os.iso
  
  # CD
  sudo umount /dev/sr0
  sudo wodim -v dev=/dev/sr0 blank=fast
  sudo wodim -v -sao speed=4 dev=/dev/sr0 build/os.iso


