20260124
# main関数に渡す引数の使い方おさらい

argcには引数の個数(argv[0]を含む為、実際の個数＋1)、argv[]には引数が入る。

```test.c
#include <stdio.h>

int main(int argc, char *argv[])
{
    int i;

    printf("argc = %d\n", argc);

    for (i = 0; i < argc; i++) {
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }

    return 0;
}
```

```bash
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc test.c -o test
test@test-fujitsu:~/kaihatsu/ctoasm$ ./test aaa bbb 123
argc = 4
argv[0] = "./test"
argv[1] = "aaa"
argv[2] = "bbb"
argv[3] = "123"
```

# 逆アセンブリ

機械語を簡潔にする為、標準ライブラリは消します。

```test.c
int main(int argc, char **argv)
{
    volatile char *sink; // argv[i]の参照を機械語で確認する為にこの変数へ格納する。未使用。

    for (int i = 0; i < argc; i++) {
        char *p = argv[i];

        while (*p != 0) {
            sink = p;
            p++;
        }
    }
    return 0;
}

```

```
test@test-fujitsu:~/kaihatsu/ctoasm$ gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -O0 -g test.c -o test
/usr/bin/ld: 警告: 无法找到项目符号 _start; 缺省为 0000000000001000
test@test-fujitsu:~/kaihatsu/ctoasm$ objdump -d -M intel test

test：     文件格式 elf32-i386


Disassembly of section .text:

00001000 <main>:
    1000:	55                   	push   ebp
    1001:	89 e5                	mov    ebp,esp
    1003:	83 ec 10             	sub    esp,0x10
    1006:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
    100d:	eb 2e                	jmp    103d <main+0x3d>
    100f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
    1012:	8d 14 85 00 00 00 00 	lea    edx,[eax*4+0x0]
    1019:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
    101c:	01 d0                	add    eax,edx
    101e:	8b 00                	mov    eax,DWORD PTR [eax]
    1020:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
    1023:	eb 0a                	jmp    102f <main+0x2f>
    1025:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
    1028:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax
    102b:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
    102f:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
    1032:	0f b6 00             	movzx  eax,BYTE PTR [eax]
    1035:	84 c0                	test   al,al
    1037:	75 ec                	jne    1025 <main+0x25>
    1039:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
    103d:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
    1040:	3b 45 08             	cmp    eax,DWORD PTR [ebp+0x8]
    1043:	7c ca                	jl     100f <main+0xf>
    1045:	b8 00 00 00 00       	mov    eax,0x0
    104a:	c9                   	leave  
    104b:	c3                   	ret    
test@test-fujitsu:~/kaihatsu/ctoasm$ 

```

# 生成された機械語の解析

ローカル変数用に16バイト(0x10)確保
```
    1000:	55                   	push   ebp
    1001:	89 e5                	mov    ebp,esp
    1003:	83 ec 10             	sub    esp,0x10
```

ローカル変数`i`を0で初期化してforの条件判定処理へ跳ぶ
```
    1006:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0 ; i=0
    100d:	eb 2e                	jmp    103d <main+0x3d>        ; for条件へ
```

p = argv[i];
```
    100f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]     ; eax = i
    1012:	8d 14 85 00 00 00 00 	lea    edx,[eax*4+0x0]             ; edx = i*4
    1019:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]     ; eax = argv
    101c:	01 d0                	add    eax,edx                     ; eax = &argv[i] (argvの先頭＋i*4)
    101e:	8b 00                	mov    eax,DWORD PTR [eax]         ; eax = argv[i]
    1020:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
```    
    
while (*p != 0)へ跳ぶ
```
    1023:	eb 0a                	jmp    102f <main+0x2f> ; while条件へ
```

sink = p;p++; 
```
    1025:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]      ; eax = p
    1028:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax      ; sink = p
    102b:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1      ; p++
```

条件判定  while (*p != 0)
``` 
    102f:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]      ; eax = p
    1032:	0f b6 00             	movzx  eax,BYTE PTR [eax]           ; eax = *p
    1035:	84 c0                	test   al,al
    1037:	75 ec                	jne    1025 <main+0x25>
```

i++
```
    1039:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
```

条件判定  i < argc
```
    103d:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]      ; eax = i
    1040:	3b 45 08             	cmp    eax,DWORD PTR [ebp+0x8]      ; i < argc
    1043:	7c ca                	jl     100f <main+0xf>
```
    
return 0;
```
    1045:	b8 00 00 00 00       	mov    eax,0x0
    104a:	c9                   	leave  
    104b:	c3                   	ret    
```

# スタック配置 

```
高アドレス
────────────
ebp+0x0c  argv
ebp+0x08  argc
ebp+0x04  戻りアドレス
ebp+0x00  保存された ebp
────────────
ebp-0x04  i
ebp-0x08  p
ebp-0x0c  sink
────────────
低アドレス
```

