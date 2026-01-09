nasm -f bin boot.asm -o boot.bin
nasm -f bin test.asm -o test.bin
cat boot.bin test.bin > disk.img 
qemu-system-i386 -hda disk.img -serial stdio
