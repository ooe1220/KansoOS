# 目的
C言語で関数を呼び出した場合、スタックがどのように積まれるかを検証します。
※ 出来るだけ純粋な状態で確認したいので自作OS上で動作させたかったのですが、デバッグが面倒なので32ビットLinuxで検証します。

# Cソース

```test.c
void test(char c, int i) {
    asm volatile("nop");
}

int main() {
    char c = 0x12;
    int i = 0x3456789a;
    test(c, i);
    return 0;
}
```

# GDB操作

gcc -m32 -fno-pie -no-pie -O0 -o test test.c
-fno-pie → コンパイル時に PIE を作らない
-no-pie → リンカで PIE を作らない

```
gcc -m32 -g -O0 test.c -o test
gdb ./test
```

GDBの命令
```
set disassembly-flavor intel
disassemble main
disassemble test
break test
run
x/24xb $esp
info registers esp
```

(gdb) backtrace
#0  test (c=18 '\022', i=878082202) at test.c:22
#1  0x565561cf in main () at test.c:28


# 機械語及びスタックを確認

```
(gdb) disassemble main
Dump of assembler code for function main:
   0x08049176 <+0>:	push   ebp
   0x08049177 <+1>:	mov    ebp,esp
   0x08049179 <+3>:	sub    esp,0x10
   0x0804917c <+6>:	mov    BYTE PTR [ebp-0x5],0x12
   0x08049180 <+10>:	mov    DWORD PTR [ebp-0x4],0x3456789a
   0x08049187 <+17>:	movsx  eax,BYTE PTR [ebp-0x5]
   0x0804918b <+21>:	push   DWORD PTR [ebp-0x4]
   0x0804918e <+24>:	push   eax
   0x0804918f <+25>:	call   0x8049166 <test>
   0x08049194 <+30>:	add    esp,0x8
   0x08049197 <+33>:	mov    eax,0x0
   0x0804919c <+38>:	leave  
   0x0804919d <+39>:	ret    
End of assembler dump.
(gdb) disassemble test
Dump of assembler code for function test:
   0x08049166 <+0>:	push   ebp
   0x08049167 <+1>:	mov    ebp,esp
   0x08049169 <+3>:	sub    esp,0x4
   0x0804916c <+6>:	mov    eax,DWORD PTR [ebp+0x8]
   0x0804916f <+9>:	mov    BYTE PTR [ebp-0x4],al
   0x08049172 <+12>:	nop
   0x08049173 <+13>:	nop
   0x08049174 <+14>:	leave  
   0x08049175 <+15>:	ret    
End of assembler dump.
```

```
(gdb) info registers esp
esp            0xffffd1a4          0xffffd1a4
```

```
(gdb) x/24xb $esp
0xffffd1a4:	0x6c	0xe6	0xfb	0xf7	0xc8	0xd1	0xff	0xff
0xffffd1ac:	0x94	0x91	0x04	0x08	0x12	0x00	0x00	0x00
0xffffd1b4:	0x9a	0x78	0x56	0x34	0x00	0xe0	0xf9	0xf7
```

メモリ

アドレスを4バイトで割り切れるように配置するため、必ずしも変数が隣接するという訳ではないようです。
```
0xffffd1a4: 0x6c
0xffffd1a5: 0xe6
0xffffd1a6: 0xfb
0xffffd1a7: 0xf7
0xffffd1a8: 0xc8
0xffffd1a9: 0xd1
0xffffd1aa: 0xff
0xffffd1ab: 0xff

0xffffd1ac: 0x94 <<< 戻りアドレス (callによって積まれる)
0xffffd1ad: 0x91
0xffffd1ae: 0x04
0xffffd1af: 0x08 <<< 戻りアドレス
0xffffd1b0: 0x12 <<< char c
0xffffd1b1: 0x00
0xffffd1b2: 0x00
0xffffd1b3: 0x00

0xffffd1b4: 0x9a <<< int i
0xffffd1b5: 0x78
0xffffd1b6: 0x56
0xffffd1b7: 0x34 <<< int i
0xffffd1b8: 0x00
0xffffd1b9: 0xe0
0xffffd1ba: 0xf9
0xffffd1bb: 0xf7
```
