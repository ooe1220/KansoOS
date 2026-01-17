20260117
# 初めに
C言語では文字列を関数に渡す処理が良く出てきますが、機械語では実際にどのようにして関数へ渡しているのか気になった為、検証することにしました。

フラットバイナリにしてしまうとセクション情報が消えて、逆アセンブルした際に全て機械語として変換されます。
文字列が探しづらい為、ELFを使用します。

# 検証に使用するコマンド

```bash
# 最適化無効
gcc -m32 -ffreestanding -fno-pic -fno-pie -O0 -c test.c -o test.o

# 最適化有効
gcc -m32 -ffreestanding -fno-pic -fno-pie -O2 -c test.c -o test.o


# リンク（簡単に0x8000から開始）
ld -m elf_i386 \
   -T linker.ld \
   -nostdlib \
   -static \
   -o test.elf test.o

objdump -d -Mintel test.elf
objdump -s -j .data test.elf
objdump -s -j .rodata test.elf
```

```linker.ld
ENTRY(foo) # 実行開始する関数

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

# func("STRING")

引数の中に文字列をそのまま書いた場合、どのように関数に渡されるのかを確認します。

```
void foo(char *s) {
    s[0]='X';
    s[1]='Y';
}


void main(void) {
    foo("ABC");
}
```

## 最適化無し(-O0)

アドレス`8028`から文字列が配置してあり、`call`で`foo()`を呼ぶ前に`push   0x8028`を実行してスタックに**文字列の先頭アドレス**を保存しています。


```
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -Mintel test.elf
objdump -s -j .data test.elf
objdump -s -j .rodata test.elf

test.elf：     文件格式 elf32-i386


Disassembly of section .text:

00008000 <foo>:
    8000:	55                   	push   ebp
    8001:	89 e5                	mov    ebp,esp
    8003:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    8006:	c6 00 58             	mov    BYTE PTR [eax],0x58
    8009:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    800c:	83 c0 01             	add    eax,0x1
    800f:	c6 00 59             	mov    BYTE PTR [eax],0x59
    8012:	90                   	nop
    8013:	5d                   	pop    ebp
    8014:	c3                   	ret    

00008015 <main>:
    8015:	55                   	push   ebp
    8016:	89 e5                	mov    ebp,esp
    8018:	68 28 80 00 00       	push   0x8028
    801d:	e8 de ff ff ff       	call   8000 <foo>
    8022:	83 c4 04             	add    esp,0x4
    8025:	90                   	nop
    8026:	c9                   	leave  
    8027:	c3                   	ret    

test.elf：     文件格式 elf32-i386

objdump: section '.data' mentioned in a -j option, but not found in any input file

test.elf：     文件格式 elf32-i386

Contents of section .rodata:
 8028 41424300                             ABC.            
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

## 最適化有り(-O2)

`foo()`字体は残っていますが、関数呼び出しそのものをしなくなりました。
結果が決まっている場合はマクロと同じような扱いになるようです。

```
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -Mintel test.elf
objdump -s -j .data test.elf
objdump -s -j .rodata test.elf

test.elf：     文件格式 elf32-i386


Disassembly of section .text:

00008000 <foo>:
    8000:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
    8004:	ba 58 59 00 00       	mov    edx,0x5958
    8009:	66 89 10             	mov    WORD PTR [eax],dx
    800c:	c3                   	ret    
    800d:	66 90                	xchg   ax,ax
    800f:	90                   	nop

00008010 <main>:
    8010:	b8 58 59 00 00       	mov    eax,0x5958
    8015:	66 a3 1c 80 00 00    	mov    ds:0x801c,ax
    801b:	c3                   	ret    

test.elf：     文件格式 elf32-i386

objdump: section '.data' mentioned in a -j option, but not found in any input file

test.elf：     文件格式 elf32-i386

Contents of section .rodata:
 801c 41424300                             ABC.            
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

# func(p)

続いてポインタ渡しの場合はどうなるかを確認します。

※参考：ASCII
　`0x58`=X
　`0x59`=Y

## 最適化無し(-O0)

レジスタ経由ではありますが、文字列の先頭アドレスをスタックに積む処理は同じです。

```
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -Mintel test.elf
objdump -s -j .data test.elf
objdump -s -j .rodata test.elf

test.elf：     文件格式 elf32-i386


Disassembly of section .text:

00008000 <foo>:
    8000:	55                   	push   ebp
    8001:	89 e5                	mov    ebp,esp
    8003:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    8006:	c6 00 58             	mov    BYTE PTR [eax],0x58
    8009:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    800c:	83 c0 01             	add    eax,0x1
    800f:	c6 00 59             	mov    BYTE PTR [eax],0x59
    8012:	90                   	nop
    8013:	5d                   	pop    ebp
    8014:	c3                   	ret    

00008015 <main>:
    8015:	55                   	push   ebp
    8016:	89 e5                	mov    ebp,esp
    8018:	83 ec 10             	sub    esp,0x10
    801b:	c7 45 fc 30 80 00 00 	mov    DWORD PTR [ebp-0x4],0x8030
    8022:	ff 75 fc             	push   DWORD PTR [ebp-0x4]
    8025:	e8 d6 ff ff ff       	call   8000 <foo>
    802a:	83 c4 04             	add    esp,0x4
    802d:	90                   	nop
    802e:	c9                   	leave  
    802f:	c3                   	ret    

test.elf：     文件格式 elf32-i386

objdump: section '.data' mentioned in a -j option, but not found in any input file

test.elf：     文件格式 elf32-i386

Contents of section .rodata:
 8030 41424300                             ABC. 
```

## 最適化有り(-O2)

関数呼び出しが消えました。
```
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -Mintel test.elf
objdump -s -j .data test.elf
objdump -s -j .rodata test.elf

test.elf：     文件格式 elf32-i386


Disassembly of section .text:

00008000 <foo>:
    8000:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
    8004:	ba 58 59 00 00       	mov    edx,0x5958
    8009:	66 89 10             	mov    WORD PTR [eax],dx
    800c:	c3                   	ret    
    800d:	66 90                	xchg   ax,ax
    800f:	90                   	nop

00008010 <main>:
    8010:	b8 58 59 00 00       	mov    eax,0x5958
    8015:	66 a3 1c 80 00 00    	mov    ds:0x801c,ax
    801b:	c3                   	ret    

test.elf：     文件格式 elf32-i386

objdump: section '.data' mentioned in a -j option, but not found in any input file

test.elf：     文件格式 elf32-i386

Contents of section .rodata:
 801c 41424300                             ABC.            
test@test-fujitsu:~/kaihatsu/ctoasm$ 

```



