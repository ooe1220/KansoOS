
# 初回
```
sudo curl -L -o /usr/share/bochs/BIOS-bochs-latest \
  https://github.com/bochs-emu/Bochs/raw/master/bios/BIOS-bochs-latest
  
sudo chmod 644 /usr/share/bochs/BIOS-bochs-latest
ls -l /usr/share/bochs/BIOS-bochs-latest
```

```
sudo apt install vgabios -y
find /usr -name "vgabios.bin" 2>/dev/null
```

```
sudo rm /usr/share/bochs/VGABIOS-lgpl-latest
sudo cp /usr/share/vgabios/vgabios.bin /usr/share/bochs/VGABIOS-lgpl-latest

ls -l /usr/share/bochs/VGABIOS-lgpl-latest
```

# 起動

```
rm build/disk.img.lock
echo 'c' > /tmp/autoc.txt
bochs -f bochsrc.txt -q
```

rm -f build/disk.img.lock
echo 'c' > /tmp/autoc.txt
bochs -f bochsrc.txt -q -rc /tmp/autoc.txt

# 固まった時

```
pkill -9 bochs
```
