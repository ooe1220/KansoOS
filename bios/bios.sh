nasm -f bin mybios.asm -o mybios.bin
qemu-system-i386   -bios mybios.bin   -vga std   -no-reboot   -no-shutdown   -serial stdio



qemu-system-i386 \
  -bios mybios.bin \
  -drive file=bootsector.bin,format=raw,if=floppy \
  -vga std \
  -no-reboot \
  -no-shutdown \
  -serial stdio
