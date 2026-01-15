clear
nasm -f bin mybios.asm -o ../build/mybios.bin
nasm -f bin bootsector.asm -o ../build/bootsector.bin
  
  qemu-system-i386 \
  -bios ../build/mybios.bin \
  -drive file=../build/bootsector.bin,format=raw,if=ide,index=0 \
  -monitor stdio
  # -serial stdio

# 確認用ブートローダをQEMU標準で立ち上げる
# qemu-system-i386 ../build/bootsector.bin
