nasm -f bin boot.asm -o ../build/boot.bin
nasm -f bin test.asm -o ../build/test.bin
cat ../build/boot.bin ../build/test.bin > ../build/disk.img 

qemu-system-i386 \
  -drive file=../build/disk.img,format=raw \
  -serial stdio

#ndisasm -b 16 ../build/test.bin
