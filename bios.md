
# 目的

自作OSを開発しています。
実機で起動する際に実機BIOSが動いた後。自分で基板上のチップやメモリを再設定したいと思います。
そこで筆者の所有しているパソコンのチップ等の情報を詳しく調べます。

対象機種:dynabook M10 11L/2

# Linuxから基板上のチップの情報を取得

```
sudo sh -c "cd \"$PWD\" && lspci -nn > pci_nn.txt 2>/dev/null"
sudo sh -c "cd \"$PWD\" && cat /proc/iomem > iomem.txt"
sudo sh -c "cd \"$PWD\" && lspci -nn 2>/dev/null | grep -iE 'host bridge|isa bridge|lpc|vga' > pci_bridge.txt"
sudo sh -c "cd \"$PWD\" && cat /proc/cpuinfo > cpuinfo.txt"
sudo sh -c "cd \"$PWD\" && ( [ -d /sys/firmware/memmap ] && find /sys/firmware/memmap -type f -exec sh -c 'echo == {}; cat {}' \; > e820.txt || dmesg | grep -iE 'e820|BIOS-e820' > e820.txt )"
```

# 北のチップ

https://www.intel.com/content/dam/doc/datasheet/852gm-852gmv-chipset-graphics-memory-controller-hub-datasheet.pdf

```pci_bridge.txt
00:00.0 Host bridge [0600]: Intel Corporation 82852/82855 GM/GME/PM/GMV Processor to I/O Controller [8086:3580] (rev 02)
00:02.0 VGA compatible controller [0300]: Intel Corporation 82852/855GM Integrated Graphics Device [8086:3582] (rev 02)
```

# 南のチップ

https://www.intel.cn/content/dam/doc/datasheet/82801dbm-i-o-controller-hub-4-mobile-datasheet.pdf
https://www.intel.cn/content/dam/doc/specification-update/82801dbm-io-controller-hub-4-mobile-specification-update.pdf

```pci_bridge.txt
00:1f.0 ISA bridge [0601]: Intel Corporation 82801DBM (ICH4-M) LPC Interface Bridge [8086:24cc] (rev 03)
```

# CPUデータシート
https://www.intel.com/content/www/us/en/content-details/841542/intel-pentium-m-processor-datasheet.html
https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

根拠
```cpuinfo.txt
model name	: Intel(R) Pentium(R) M processor 1.10GHz
```

