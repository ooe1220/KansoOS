
そのまま貼り付ける。

# intの中身16進数で表示

```c
volatile unsigned short *vram = (unsigned short*)0xB8000;
unsigned char hex[] = "0123456789ABCDEF";

int val = 0xDEADBEEF;
int x = 0, y = 0;
for(int i=0;i<8;i++){
    int d = (val >> (28 - i*4)) & 0xF;
    vram[y*80 + x + i] = (0x0F << 8) | hex[d];
}
```

# ポインタ中のアドレスを16進数で表示

```c
volatile unsigned short *vram = (unsigned short*)0xB8000;
unsigned char hex[] = "0123456789ABCDEF";

int some_var = 1234;
void *ptr = &some_var;
uint32_t val = (uint32_t)ptr;  // ptrは任意の32bitアドレス
int x = 0, y = 0;

for(int i = 0; i < 8; i++) {
    int d = (val >> (28 - i*4)) & 0xF;
    vram[y*80 + x + i] = (0x0F << 8) | hex[d];
}
```

# EBPを基準にスタックの中身を表示する

```c
    volatile unsigned short *vram = (unsigned short*)0xB8000;
    unsigned char hex[] = "0123456789ABCDEF";

    uint32_t *bp;
    asm volatile("mov %%ebp, %0" : "=r"(bp)); // EBPを取得

    int x = 0, y = 0;

    // --- EBPの値を1行目に表示 ---
    char *label = "EBP=";
    for(int i = 0; label[i]; i++)
        vram[y*80 + x + i] = (0x0F << 8) | label[i];

    x += 4; // ラベル分だけ右にずらす
    uint32_t ebp_val = (uint32_t)bp;
    for(int j = 0; j < 8; j++){
        int d = (ebp_val >> (28 - j*4)) & 0xF;
        vram[y*80 + x + j] = (0x0F << 8) | hex[d];
    }

    y++;    // 次の行
    x = 0;  // 行先頭に戻す

    // --- スタックダンプ（高アドレス順） ---
    // 引数(3)からローカル変数(-2)まで逆順に表示
    for(int i = 3; i >= -2; i--){
        uint32_t addr = (uint32_t)&bp[i]; // アドレス
        uint32_t val  = bp[i];            // 値

        // アドレスを8桁16進で表示
        for(int j = 0; j < 8; j++){
            int d = (addr >> (28 - j*4)) & 0xF;
            vram[y*80 + x + j] = (0x0F << 8) | hex[d];
        }

        vram[y*80 + x + 8] = (0x0F << 8) | ':'; // 区切り

        // 値を8桁16進で表示
        for(int j = 0; j < 8; j++){
            int d = (val >> (28 - j*4)) & 0xF;
            vram[y*80 + x + 9 + j] = (0x0F << 8) | hex[d];
        }

        y++; // 次の行へ
    }
```

[EBP-4]が返りアドレスとなる。
返りアドレス-5(callの命令長)を`x/10i 0xXXXX`のXXXXに入れると`call`命令が見えるはず。

# 到達確認

```
    mov edi, 0xB8000
    mov byte ptr [edi], 'A'
    mov byte ptr [edi+1], 0x0F
```

```c
    volatile unsigned char* vram = (unsigned char*)0xB8000;
    vram[4] = 'C';      // 文字
    vram[5] = 0x0F;     // 白文字・黒背景
```

# hlt→GDB

```c
    asm volatile("cli");
    asm volatile("hlt");
```

```
info registers 
x/4xw $esp 
```

# メモリ上のプログラム逆アセンブル

```
x/10i 0x9fa90
```


初めのHLT
EBP=0009fb38 ESP=0009fa7c
0009fa7c: 0x00008a9f 0x00010000 0x00000001 0x0009fa90

返りのHLT
EBP=0009fb38 ESP=0009fa7c
0009fa7c: 0x00008a9f 0x00010000 0x00000001 0x0009fa90



