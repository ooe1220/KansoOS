20260124

# 目的
自作OSのソースファイル量を減らすために、一部の関数は`.c`ではなく`.h`ファイルの中で直に`static`関数を実装していました。

しかしコンパイル後のファイルが妙に大きいことに気づき、逆アセンブルした結果を見てみると、呼び出し側のファイル内に関数が展開されることが判明しました。
そこで今回は以下の検証を行います。

1. `test.c`から`lib.h`(ヘッダファイル内で`libfunc`関数を実装)を呼び出して、`test.o`内に`libfunc`が生成されることを確認→生成された
2. `test1.c`、`test2.c`から`lib.h`を呼び出した後にリンクするとオブジェクトファイル内に`libfunc`関数が２つ生成されるかを確認→生成された
3. `lib.c`内に実装し、`lib.h`経由で関数を呼び出し(別cファイル上の関数を呼び出す際は`static`には出来ない)、`test.o`内に`libfunc`が生成されないことを確認。→生成されなかった

# 検証
関数が呼び出し元に生成された原因は恐らく`static`をつけたから。
他の`.c`ファイル中の関数を呼び出す際は`static`に設定できない。
`.h`ファイルの中に関数を実装する場合は`static`が必要。

ソースの量を減らす為に`.h`ファイル内に実装してきたが、その場合はバイナリ容量が増えてしまうので、場合に応じて取捨選択が必要かもしてない。

# lib.hの中に関数を実装する

```lib.h
static int libfunc(int x) {
    return x + 1;
}
```

# `test.o`内に`libfunc`が生成されることを確認

```test.c
#include "lib.h"

int main(void) {
    return libfunc(3);
}
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g test.c -o test.o
/usr/bin/ld: 警告: 无法找到项目符号 _start; 缺省为 0000000000001000
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -M intel test.o

test.o：     文件格式 elf32-i386


Disassembly of section .text:

00001000 <libfunc>:
    1000:	55                   	push   ebp
    1001:	89 e5                	mov    ebp,esp
    1003:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    1006:	83 c0 01             	add    eax,0x1
    1009:	5d                   	pop    ebp
    100a:	c3                   	ret    

0000100b <main>:
    100b:	55                   	push   ebp
    100c:	89 e5                	mov    ebp,esp
    100e:	6a 03                	push   0x3
    1010:	e8 eb ff ff ff       	call   1000 <libfunc>
    1015:	83 c4 04             	add    esp,0x4
    1018:	c9                   	leave  
    1019:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

# `libfunc`関数が２つ生成されるかを確認

```test1.c
#include "lib.h"

int main(void) {
    return libfunc(10);
}
```

```test2.c
#include "lib.h"

int test2(void) {
    return libfunc(20);
}
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g -c test1.c -o test1.o
gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g -c test2.c -o test2.o 
objdump -d -M intel test1.o
objdump -d -M intel test2.o

test1.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <libfunc>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   6:	83 c0 01             	add    eax,0x1
   9:	5d                   	pop    ebp
   a:	c3                   	ret    

0000000b <main>:
   b:	55                   	push   ebp
   c:	89 e5                	mov    ebp,esp
   e:	6a 0a                	push   0xa
  10:	e8 eb ff ff ff       	call   0 <libfunc>
  15:	83 c4 04             	add    esp,0x4
  18:	c9                   	leave  
  19:	c3                   	ret    

test2.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <libfunc>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   6:	83 c0 01             	add    eax,0x1
   9:	5d                   	pop    ebp
   a:	c3                   	ret    

0000000b <test2>:
   b:	55                   	push   ebp
   c:	89 e5                	mov    ebp,esp
   e:	6a 14                	push   0x14
  10:	e8 eb ff ff ff       	call   0 <libfunc>
  15:	83 c4 04             	add    esp,0x4
  18:	c9                   	leave  
  19:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

リンクして1つのオブジェクトファイルにする
```
test@test-fujitsu:~/kaihatsu/ctoasm$ ld -m elf_i386 -r test1.o test2.o -o combined.o
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -M intel combined.o

combined.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <libfunc>:
   0:	55                   	push   ebp
   1:	89 e5                	mov    ebp,esp
   3:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   6:	83 c0 01             	add    eax,0x1
   9:	5d                   	pop    ebp
   a:	c3                   	ret    

0000000b <main>:
   b:	55                   	push   ebp
   c:	89 e5                	mov    ebp,esp
   e:	6a 0a                	push   0xa
  10:	e8 eb ff ff ff       	call   0 <libfunc>
  15:	83 c4 04             	add    esp,0x4
  18:	c9                   	leave  
  19:	c3                   	ret    

0000001a <libfunc>:
  1a:	55                   	push   ebp
  1b:	89 e5                	mov    ebp,esp
  1d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  20:	83 c0 01             	add    eax,0x1
  23:	5d                   	pop    ebp
  24:	c3                   	ret    

00000025 <test2>:
  25:	55                   	push   ebp
  26:	89 e5                	mov    ebp,esp
  28:	6a 14                	push   0x14
  2a:	e8 eb ff ff ff       	call   1a <libfunc>
  2f:	83 c4 04             	add    esp,0x4
  32:	c9                   	leave  
  33:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```


# `lib.c`内に実装し、`lib.h`経由で関数を呼び出す

```test.c
#include "lib.h"

int main(void) {
    return libfunc(10);
}
```

```lib.c
#include "lib.h"

int libfunc(int x) {
    return x + 1;
}
```

```lib.h
#ifndef LIB_H
#define LIB_H

int libfunc(int x);

#endif
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g -c test.c -o test.o
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g lib.c -o lib.o
/usr/bin/ld: 警告: 无法找到项目符号 _start; 缺省为 0000000000001000
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -M intel test.o

test.o：     文件格式 elf32-i386


Disassembly of section .text:

00000000 <main>:
   0:	8d 4c 24 04          	lea    ecx,[esp+0x4]
   4:	83 e4 f0             	and    esp,0xfffffff0
   7:	ff 71 fc             	push   DWORD PTR [ecx-0x4]
   a:	55                   	push   ebp
   b:	89 e5                	mov    ebp,esp
   d:	51                   	push   ecx
   e:	83 ec 04             	sub    esp,0x4
  11:	83 ec 0c             	sub    esp,0xc
  14:	6a 0a                	push   0xa
  16:	e8 fc ff ff ff       	call   17 <main+0x17>
  1b:	83 c4 10             	add    esp,0x10
  1e:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  21:	c9                   	leave  
  22:	8d 61 fc             	lea    esp,[ecx-0x4]
  25:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -M intel lib.o

lib.o：     文件格式 elf32-i386


Disassembly of section .text:

00001000 <libfunc>:
    1000:	55                   	push   ebp
    1001:	89 e5                	mov    ebp,esp
    1003:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
    1006:	83 c0 01             	add    eax,0x1
    1009:	5d                   	pop    ebp
    100a:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 
```

