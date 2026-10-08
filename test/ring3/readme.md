
```
mkdir tmp

nasm -f elf32 start.asm -o tmp/start.o
gcc -m32 -ffreestanding -nostdlib -c kernel.c -o tmp/kernel.o
gcc -m32 -ffreestanding -nostdlib -c gdt.c -o tmp/gdt.o
ld -m elf_i386 -T link.ld -o tmp/kernel.bin tmp/start.o tmp/kernel.o tmp/gdt.o

grub-file --is-x86-multiboot tmp/kernel.bin

mkdir -p iso/boot/grub
cp tmp/kernel.bin iso/boot/kernel.bin
cp grub.cfg iso/boot/grub/grub.cfg
grub-mkrescue -o tmp/myos.iso iso

qemu-system-i386 -cdrom tmp/myos.iso
```

qemu-system-i386 -no-reboot -cdrom tmp/myos.iso
qemu-system-i386 -cdrom tmp/myos.iso -d int -D qemu.log
