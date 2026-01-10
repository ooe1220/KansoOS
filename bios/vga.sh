nasm -f bin boot.asm -o ../build/boot.bin
nasm -f bin test.asm -o ../build/test.bin
cat ../build/boot.bin ../build/test.bin > ../build/disk.img 
qemu-system-i386 -hda ../build/disk.img -serial stdio
