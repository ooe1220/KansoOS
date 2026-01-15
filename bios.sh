clear
nasm -f bin bios/mybios.asm -o build/mybios.bin
nasm -f bin bios/bootsector.asm -o build/bootsector.bin

# ./build.sh
  
   qemu-system-i386 \
   -bios build/mybios.bin \
   -drive file=build/bootsector.bin,format=raw,if=ide,index=0 \
   -monitor stdio
   #-serial stdio

# 確認用ブートローダをQEMU標準で立ち上げる
# qemu-system-i386 ../build/bootsector.bin

# 自作OSを立ち上げる
#   qemu-system-i386 \
#   -bios build/mybios.bin\
#   -drive file=build/disk.img,format=raw,if=ide,index=0 \
#   -monitor stdio
