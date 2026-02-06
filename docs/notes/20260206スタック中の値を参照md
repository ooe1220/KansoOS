20260117

# 初めに
FLFファイルの各セクションの意味を調べます。
簡素OSでは最終的にフラットバイナリに変換しますが、コンパイル途中で一度ELFを介すので仕様を理解して置きます。

# 検証コマンド

```
gcc -m32 -ffreestanding -fno-pic -fno-pie -O0 -c test.c -o test.o

readelf -S test.o
objdump -s -j .data test.o
objdump -s -j .rodata test.o
objdump -D -j .text test.o -M intel
```


# 各セクションの用途

.text : 機械語コードが置かれる
.data : 初期値がある広域変数
.bss : 初期値がない広域変数
.rodata : 読み取り専用。書き換えを禁止する定数、文字列リテラル（`printf("Hello");`の"Hello"等）

# ソース

```test.c
/* .data セクション: 初期化済み広域変数 */
int initialized_var = 42;

/* .bss セクション: 未初期化広域変数 */
int uninitialized_var;

/* const有セクション:rodata 無:dataセクション */
const char message[] = "Hello, ELF!";

void foo(char *s);

/* .text セクション: 実行コード */
void test_main() {
    initialized_var += 1;
    uninitialized_var = 100;

    volatile const char *msg = message;
    (void)msg;
    
    foo("ABC");
}

void foo(char *s) {
    s[0]='X';
    s[1]='Y';
}
```

# 各セクションの値を確認

```
test@test-fujitsu:~/kaihatsu/ctoasm$ readelf -S test.o
objdump -s -j .data test.o
objdump -s -j .rodata test.o
objdump -D -j .text test.o -M intel
There are 13 section headers, starting at offset 0x27c:

节头：
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        00000000 000034 00004c 00  AX  0   0  1
  [ 2] .rel.text         REL             00000000 0001dc 000030 08   I 10   1  4
  [ 3] .data             PROGBITS        00000000 000080 000004 00  WA  0   0  4
  [ 4] .bss              NOBITS          00000000 000084 000004 00  WA  0   0  4
  [ 5] .rodata           PROGBITS        00000000 000084 000010 00   A  0   0  4
  [ 6] .comment          PROGBITS        00000000 000094 00002e 01  MS  0   0  1
  [ 7] .note.GNU-stack   PROGBITS        00000000 0000c2 000000 00      0   0  1
  [ 8] .eh_frame         PROGBITS        00000000 0000c4 000058 00   A  0   0  4
  [ 9] .rel.eh_frame     REL             00000000 00020c 000010 08   I 10   8  4
  [10] .symtab           SYMTAB          00000000 00011c 000090 10     11   4  4
  [11] .strtab           STRTAB          00000000 0001ac 000030 00      0   0  1
  [12] .shstrtab         STRTAB          00000000 00021c 00005f 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), p (processor specific)

test.o：     文件格式 elf32-i386

Contents of section .data:
 0000 2a000000                             *...            

test.o：     文件格式 elf32-i386

Contents of section .rodata:
 0000 48656c6c 6f2c2045 4c462100 41424300  Hello, ELF!.ABC.

test.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <test_main>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	83 ec 18             	sub    esp,0x18
   6:	a1 00 00 00 00       	mov    eax,ds:0x0
   b:	83 c0 01             	add    eax,0x1
   e:	a3 00 00 00 00       	mov    ds:0x0,eax
  13:	c7 05 00 00 00 00 64 	mov    DWORD PTR ds:0x0,0x64
  1a:	00 00 00 
  1d:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [ebp-0xc],0x0
  24:	83 ec 0c             	sub    esp,0xc
  27:	68 0c 00 00 00       	push   0xc
  2c:	e8 fc ff ff ff       	call   2d <test_main+0x2d>
  31:	83 c4 10             	add    esp,0x10
  34:	90                   	nop
  35:	c9                   	leave  
  36:	c3                   	ret    

00000037 <foo>:
  37:	55                   	push   ebp
  38:	89 e5                	mov    ebp,esp
  3a:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  3d:	c6 00 58             	mov    BYTE PTR [eax],0x58
  40:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  43:	83 c0 01             	add    eax,0x1
  46:	c6 00 59             	mov    BYTE PTR [eax],0x59
  49:	90                   	nop
  4a:	5d                   	pop    ebp
  4b:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

`int initialized_var = 42;`は初期値のある広域変数、よって`.data`に配置されている
```
Contents of section .data:
 0000 2a000000                             *...   
```

`const char message[] = "Hello, ELF!";`及び`foo("ABC");`の文字列は読み取り専用、よって`.rodata`に配置されている
```
Contents of section .rodata:
 0000 48656c6c 6f2c2045 4c462100 41424300  Hello, ELF!.ABC.
```

実行可能なコードは`.text`に配置されている
```
Disassembly of section .text:

00000000 <test_main>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
```

BSSは確かめる術が無い。

以下の命令から`const`(読み取り専用)を外して、`.rodata`及び`.data`を確認する
```
const char message[] = "Hello, ELF!";
```

以下の様に書き換える
```
char message[] = "Hello, ELF!";
```

すると`message[]`は読み取り専用ではなくなるので`.data`に移動した。
```
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -s -j .data test.o
objdump -s -j .rodata test.o

test.o：     文件格式 elf32-i386

Contents of section .data:
 0000 2a000000 48656c6c 6f2c2045 4c462100  *...Hello, ELF!.

test.o：     文件格式 elf32-i386

Contents of section .rodata:
 0000 41424300                             ABC.     
```

