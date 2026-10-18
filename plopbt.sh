#!/bin/bash

# https://www.plop.at/en/bootmanager/download.html

lsblk # sdXを調べる

sudo dd if=build/disk.img of=/dev/sdb bs=512 count=2048 conv=notrunc

# PLOP BOOT MANAGERの円盤を挿入していること

# USBに書き込めているか確認
sudo qemu-system-x86_64 \
  -drive file=/dev/sdb,format=raw \
  -m 2048 \
  -enable-kvm
