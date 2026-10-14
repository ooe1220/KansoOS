#!/bin/bash

# QEMUでは動くが実機では動かない
 # 64kbまで0埋め

nasm -f bin src/boot/cdboot.asm -o build/cdboot.bin

mkdir -p iso_root/boot
cp build/cdboot.bin iso_root/boot/
cp build/kernel.bin  iso_root/

 xorriso -as mkisofs \
  -o os.iso \
  -b boot/cdboot.bin \
  -no-emul-boot \
  -boot-load-size 4 \
  iso_root

  qemu-system-i386 -cdrom os.iso -boot d -monitor stdio
  # (qemu)xp /100xb 0x8000
