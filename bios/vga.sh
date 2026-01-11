#nasm -f bin boot.asm -o ../build/boot.bin
#nasm -f bin test.asm -o ../build/test.bin
#cat ../build/boot.bin ../build/test.bin > ../build/disk.img 
#qemu-system-i386 -hda ../build/disk.img -serial stdio


nasm -f bin boot.asm -o boot.bin
nasm -f bin test.asm -o test.bin
cat boot.bin test.bin > disk.img 

qemu-system-i386 \
  -drive file=disk.img,format=raw \
  -serial stdio

#qemu-system-i386 -hda disk.img -serial stdio
