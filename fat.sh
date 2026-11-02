#!/bin/bash

nasm -f bin src/boot/vbr.asm -o build/vbr.bin


#FAT16イメージ生成→mkfs.fatの生成したBPB抽出→自作VBRへ標準BPBを書き込み→自作VBRをFAT16イメージへ書き込み
dd if=/dev/zero of=build/fat16.img bs=1M count=16
mkfs.fat -F16 -s 8 -S 512 build/fat16.img
dd if=build/fat16.img of=build/vbr_until_start.bin bs=1 count=62
dd if=build/vbr_until_start.bin of=build/vbr.bin bs=1 seek=0 conv=notrunc
dd if=build/vbr.bin of=build/fat16.img bs=512 count=1 conv=notrunc

#xxd build/vbr_until_start.bin
#xxd -l 512 build/vbr.bin
#ndisasm -b 16 -o 0x7c00 -s 0x7c3e build/vbr.bin | head -60
#qemu-system-i386 -hda build/vbr.bin

echo 'drive x: file="build/fat16.img" fat_bits=16' > mtoolsrc
export MTOOLSRC=$PWD/mtoolsrc

mcopy build/kernel.bin x:KERNEL.BIN

mcopy build/test2.bin   x:TEST2.BIN
mcopy build/test3.bin   x:TEST3.BIN
mcopy build/cat.bin     x:CAT.BIN
mcopy build/memtest.bin x:MEMTEST.BIN

cp user/test.txt build/test.txt
mcopy build/test.txt x:TEST.TXT

qemu-system-i386 -hda build/fat16.img  -monitor stdio

# kernel.binの逆アセンブル　kernel.binはLBA72から始まっている
# dd if=build/fat16.img bs=512 skip=72 count=4 of=/tmp/s.bin 2>/dev/null
# ndisasm -b 16 -o 0x8000 /tmp/s.bin

# VBRのダンプ
# xxd -l 512 build/fat16.img

# VBRを逆アセンブル
# ndisasm -b 16 -o 0x7c00 -s 0x7c3e build/fat16.img | head -60

# 起動ディスク内ファイル一覧
# mdir -i "build/fat16.img" ::

## test.txt
cp user/test.txt build/test.txt