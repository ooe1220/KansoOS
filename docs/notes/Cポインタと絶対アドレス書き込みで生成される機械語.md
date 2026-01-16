# 目的

C言語でベアメタル向けのプログラムを書いていると、複数のCソースを手動でリンクすることになりますが、果たしてリンク後に本当に別ファイルの関数を呼べているのか不安になります。
そこで今回は複数のCソースから同一の関数を呼び出し、それぞれ同じアドレスを指せているかを確認します。

# ソース

```main1.c
#include "func.h"

int main1(void) {
    foo();
    return 0;
}
```

```main2.c
#include "func.h"

int main2(void) {
    foo();
    return 0;
}
```

```func.h
void foo(void);
```

```func.c
#include "func.h"

void foo(void) {
    asm volatile("nop");  // 適当な処理
}
```

```linker.ld
ENTRY(main1) # 実行開始する関数

SECTIONS {
    /* 0x8000〜の領域に読み込む想定 */
    . = 0x8000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
    
    /DISCARD/ : {
        *(.eh_frame*)
        *(.eh_frame_hdr*)
        *(.comment*)
        *(.note*)
    }
}
```


# コンパイル及び逆アセンブル

```cmp.sh
# chmod +x cmp.sh
gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -c main1.c -o main1.o
gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -c main2.c -o main2.o
gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -c func.c  -o func.o

ld -m elf_i386 -T linker.ld main1.o main2.o func.o -o kernel.elf
objcopy -O binary kernel.elf kernel.bin
objdump -D -b binary -m i386 -M intel kernel.bin
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ ./cmp.sh

kernel.bin：     文件格式 binary


Disassembly of section .data:

00000000 <.data>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	83 ec 08             	sub    esp,0x8
   6:	e8 19 00 00 00       	call   0x24
   b:	b8 00 00 00 00       	mov    eax,0x0
  10:	c9                   	leave  
  11:	c3                   	ret    
  12:	55                   	push   ebp
  13:	89 e5                	mov    ebp,esp
  15:	83 ec 08             	sub    esp,0x8
  18:	e8 07 00 00 00       	call   0x24
  1d:	b8 00 00 00 00       	mov    eax,0x0
  22:	c9                   	leave  
  23:	c3                   	ret    
  24:	55                   	push   ebp
  25:	89 e5                	mov    ebp,esp
  27:	90                   	nop
  28:	90                   	nop
  29:	5d                   	pop    ebp
  2a:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

# 考察
`call   0x24`が二つあることから、main1()及びmain2()の両方からfoo()関数を指定出来ていることが分かります。

# `call   0x24`の機械語が違う訳

`call`の機械語形式
`xx xx xx xx`は次の命令から呼び出す関数までの**相対的な**距離

```
E8 xx xx xx xx
```

foo()の開始アドレスは0x24=36
```
  24:	55                   	push   ebp
```

1個目
```
   6:	e8 19 00 00 00       	call   0x24
```
0x19=25、call自身の命令長=5
6+5+25=36


2個目
```
  18:	e8 07 00 00 00       	call   0x24
```
0x18=24
0x07=7、call自身の命令長=5
7+5+24=36



