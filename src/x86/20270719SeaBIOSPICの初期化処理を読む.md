
# 目的

前に何度かアセンブリでPICを初期化する処理を書きましたが、SeaBIOSではどう実装されているか気になったので調べてみました。
https://github.com/coreboot/seabios


自分で書いたコードは以下に乗せています。
https://github.com/ooe1220/KansoOS/tree/master/src/x86
(16ビットx86で学ぶPIC設定とタイマー割り込み)[https://qiita.com/earthen94/items/70e33c0de2242daf2ee9]

# picの初期化処理を呼び出している場所を探す

```
test@test-fujitsu:~/kaihatsu/seabios-master$ grep -R "pic_setup" .
./src/hw/pic.c:pic_setup(void)
./src/hw/pic.h:void pic_setup(void);
./src/post.c:#include "hw/pic.h" // pic_setup
./src/post.c:    pic_setup();
./src/resume.c:    pic_setup();
```

# 初期処理の流れ

https://github.com/coreboot/seabios/blob/master/src/hw/pci.c

`post.c`→`pic_setup()`→`pic_reset()`→`pic_irqmask_write()`

# 定数と関数を剥がして`outb`を全部展開する

引数の順番に注意:outb(データ, ポート)

自分がアセンブリで書いていたコードと一致しました。
```c
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, 0x0020);
    outb(0x11, 0x00a0);
    
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(0x08, 0x0021);
    outb(0x70, 0x00a1);
    
    // Send ICW3 (cascaded pic ids)
    outb(0x04, 0x0021);
    outb(0x02, 0x00a1);
    
    // Send ICW4 (enable 8086 mode)
    outb(0x01, 0x0021);
    outb(0x01, 0x00a1);
    
    outb(0xFB, 0x0021); // IMR(PIC1): IRQ2 のみ許可
    outb(0xFF, 0x00a1); // 前マスク
```





