# 目的
ポインタにアドレスを直に代入して、メモリへ値を格納した場合にどのようなコードが生成されるのかを調べます。

```test.c
void foo(void) {
    volatile unsigned char* vram = (unsigned char*)0xB8000;
    vram[0] = 'A';
    vram[1] = 0x0F;
}
```

```
gcc -m32 -ffreestanding -fno-pic -fno-pie -c test.c -o test.o

ld -m elf_i386 \
   -T linker.ld \
   -nostdlib \
   -static \
   -o test.elf test.o

objcopy -O binary test.elf test.bin

objdump -D -b binary -m i386 -M intel test.bin
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -D -b binary -m i386 -M intel test.bin

test.bin：     文件格式 binary


Disassembly of section .data:

00000000 <.data>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	83 ec 10             	sub    esp,0x10
   6:	c7 45 fc 00 80 0b 00 	mov    DWORD PTR [ebp-0x4],0xb8000
   d:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  10:	c6 00 41             	mov    BYTE PTR [eax],0x41
  13:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  16:	83 c0 01             	add    eax,0x1
  19:	c6 00 0f             	mov    BYTE PTR [eax],0xf
  1c:	90                   	nop
  1d:	c9                   	leave  
  1e:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

EAX=0xB8000
[EAX]=0x41(AのASCII)
の様になっている。
アドレスをレジスタに入れて、レジスタ経由でメモリに値を格納するコードが生成されました。

# 最適化した場合

以下の様に`-O2`オプションをつけてコンパイルした場合を見てみます。

```bash
gcc -m32 -ffreestanding -fno-pic -fno-pie -O2 -c test.c
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -D -b binary -m i386 -M intel test.bin

test.bin：     文件格式 binary


Disassembly of section .data:

00000000 <.data>:
   0:	c6 05 00 80 0b 00 41 	mov    BYTE PTR ds:0xb8000,0x41
   7:	c6 05 01 80 0b 00 0f 	mov    BYTE PTR ds:0xb8001,0xf
   e:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

絶対アドレス書き込みに変わりました。
