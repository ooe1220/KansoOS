
# 目的

今は自作OS起動ディスクのFAT16構造を自作のCプログラムから行っているが、汎用ツールで行えるようにする為、mkfs.fatで生成されるイメージを解析する。


# FATの仮想イメージを生成
```
dd if=/dev/zero of=fat16.img bs=1M count=16

mkfs.fat -F16 -s 8 -S 512 fat16.img
```

# 情報を確認
```
file fat16.img
fdisk -l fat16.img
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    file fat16.img
fat16.img: DOS/MBR boot sector, code offset 0x3c+2, OEM-ID "mkfs.fat", sectors/cluster 8, reserved sectors 8, root entries 512, sectors 32768 (volumes <=32 MB), Media descriptor 0xf8, sectors/FAT 16, sectors/track 32, serial number 0x5c30aa96, unlabeled, FAT (16 bit)

test  …/KansoOS/test/fat16   master ✘?⇡    fdisk -l fat16.img
ディスク fat16.img: 16 MiB, 16777216 バイト, 32768 セクタ
単位: セクタ (1 * 512 = 512 バイト)
セクタサイズ (論理 / 物理): 512 バイト / 512 バイト
I/O サイズ (最小 / 推奨): 512 バイト / 512 バイト
ディスクラベルのタイプ: dos
ディスク識別子: 0x00000000
```

# VBRを確認

パーテーションは切らないのでMBRは無し。

`This   is not a bootable disk.  Please  insert a bootable floppy and..press any key to try again `の文字とそれを表示する機械語か確認できる。

## ダンプを確認

```
xxd -l 512 fat16.img
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -l 512 fat16.img
00000000: eb3c 906d 6b66 732e 6661 7400 0208 0800  .<.mkfs.fat.....
00000010: 0200 0200 80f8 1000 2000 0200 0000 0000  ........ .......
00000020: 0000 0000 8000 2996 aa30 5c4e 4f20 4e41  ......)..0\NO NA
00000030: 4d45 2020 2020 4641 5431 3620 2020 0e1f  ME    FAT16   ..
00000040: be5b 7cac 22c0 740b 56b4 0ebb 0700 cd10  .[|.".t.V.......
00000050: 5eeb f032 e4cd 16cd 19eb fe54 6869 7320  ^..2.......This 
00000060: 6973 206e 6f74 2061 2062 6f6f 7461 626c  is not a bootabl
00000070: 6520 6469 736b 2e20 2050 6c65 6173 6520  e disk.  Please 
00000080: 696e 7365 7274 2061 2062 6f6f 7461 626c  insert a bootabl
00000090: 6520 666c 6f70 7079 2061 6e64 0d0a 7072  e floppy and..pr
000000a0: 6573 7320 616e 7920 6b65 7920 746f 2074  ess any key to t
000000b0: 7279 2061 6761 696e 202e 2e2e 200d 0a00  ry again ... ...
000000c0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000f0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000100: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000110: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000120: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000130: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000140: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000150: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000160: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000170: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000180: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000190: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001a0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001b0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001c0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001f0: 0000 0000 0000 0000 0000 0000 0000 55aa  ..............U.
```

## VBRの逆アセンブル

```
ndisasm -b 16 -o 0x7c00 -s 0x7c3e fat16.img | head -60
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    ndisasm -b 16 -o 0x7c00 -s 0x7c3e fat16.img | head -60
00007C00  EB3C              jmp 0x7c3e
00007C02  90                nop

＞＞＞＞ここからはデータ
00007C03  6D                insw
00007C04  6B66732E          imul sp,[bp+0x73],0x2e
00007C08  6661              popad
00007C0A  7400              jz 0x7c0c
00007C0C  0208              add cl,[bx+si]
00007C0E  0800              or [bx+si],al
00007C10  0200              add al,[bx+si]
00007C12  0200              add al,[bx+si]
00007C14  80F810            cmp al,0x10
00007C17  0020              add [bx+si],ah
00007C19  0002              add [bp+si],al
00007C1B  0000              add [bx+si],al
00007C1D  0000              add [bx+si],al
00007C1F  0000              add [bx+si],al
00007C21  0000              add [bx+si],al
00007C23  00800029          add [bx+si+0x2900],al
00007C27  96                xchg ax,si
00007C28  AA                stosb
00007C29  305C4E            xor [si+0x4e],bl
00007C2C  4F                dec di
00007C2D  204E41            and [bp+0x41],cl
00007C30  4D                dec bp
00007C31  45                inc bp
00007C32  2020              and [bx+si],ah
00007C34  2020              and [bx+si],ah
00007C36  46                inc si
00007C37  41                inc cx
00007C38  54                push sp
00007C39  31362020          xor [0x2020],si
00007C3D  20                db 0x20
＜＜＜＜ここまではデータ

＞＞＞＞ここからコード
00007C3E  0E                push cs
00007C3F  1F                pop ds
00007C40  BE5B7C            mov si,0x7c5b
00007C43  AC                lodsb
00007C44  22C0              and al,al
00007C46  740B              jz 0x7c53
00007C48  56                push si
00007C49  B40E              mov ah,0xe
00007C4B  BB0700            mov bx,0x7
00007C4E  CD10              int byte 0x10
00007C50  5E                pop si
00007C51  EBF0              jmp 0x7c43

00007C53  32E4              xor ah,ah
00007C55  CD16              int byte 0x16　　＜＜ボタン待ち
00007C57  CD19              int byte 0x19　　＜＜ボタン押されたら再起動
00007C59  EBFE              jmp 0x7c59　　　＜＜戻ってきたら無限ループ
＜＜＜＜ここまでコード

＞＞＞＞ここから文字列
00007C5B  54                push sp
00007C5C  686973            push word 0x7369
00007C5F  206973            and [bx+di+0x73],ch
00007C62  206E6F            and [bp+0x6f],ch
00007C65  7420              jz 0x7c87
00007C67  61                popa
00007C68  20626F            and [bp+si+0x6f],ah
00007C6B  6F                outsw
00007C6C  7461              jz 0x7ccf
00007C6E  626C65            bound bp,[si+0x65]
00007C71  206469            and [si+0x69],ah
00007C74  736B              jnc 0x7ce1
```

## BPBの解析

### セクタあたりバイト数 (0x0B, 2B)

`0x200 = 512`

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x0b -l 2 -g 1 fat16.img
0000000b: 00 02                                            ..
```

### セクタ/クラスタ (0x0D, 1B)

1クラスタ=8セクタ
```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x0d -l 1 -g 1 fat16.img
0000000d: 08
```

### 予約セクタ数 (0x0E, 2B)

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x0e -l 2 -g 1 fat16.img
0000000e: 08 00    
```

###  FAT 数 (0x10, 1B)

2個

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x10 -l 1 -g 1 fat16.img
00000010: 02 
```

### ルートディレクトリエントリ数 (0x11, 2B)

512個

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x11 -l 2 -g 1 fat16.img
00000011: 00 02
```

### FAT あたりのセクタ数 (0x16, 2B)

16セクタ

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x16 -l 2 -g 1 fat16.img
00000016: 10 00 
```

### 総セクタ数 32bit (0x13, 2B)

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x13 -l 2 -g 1 fat16.img
00000013: 00 80
```

### ボリュームラベル 11B (0x2B)

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x2b -l 11 fat16.img
0000002b: 4e4f 204e 414d 4520 2020 20              NO NAME
```


# 確認用にファイルを入れる

```
echo -n 'hello from test1' > /tmp/t1.txt
echo -n 'test2: second file' > /tmp/t2.txt

mcopy -i "$(pwd)/fat16.img" /tmp/t1.txt ::TEST1.TXT
mcopy -i "$(pwd)/fat16.img" /tmp/t2.txt ::TEST2.TXT
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    mdir -i "$(pwd)/fat16.img" ::
 Volume in drive : has no label
 Volume Serial Number is 5C30-AA96
Directory for ::/

TEST1    TXT        16 2026-10-31  18:51 
TEST2    TXT        18 2026-10-31  18:51 
        2 files                  34 bytes
                         16 732 160 bytes free
```

# ルートディレクトリ

```
ルートディレクトリ先頭セクタ = 予約セクタ数 + FAT数 × FATあたりセクタ数
                            = 8 + 2 × 16
                            = 40

BytesPerSector  = 512   (0x0B)
ReservedSectors = 8     (0x0E)
NumberOfFATs    = 2     (0x10)
SectorsPerFAT   = 16    (0x16)

root_start = ReservedSectors + NumberOfFATs × SectorsPerFAT
           = 8 + 2 × 16
           = 40 
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    xxd -s 0x5000 -l 128 -g 1 fat16.img
00005000: 54 45 53 54 31 20 20 20 54 58 54 20 00 00 63 96  TEST1   TXT ..c.
00005010: 5f 5d 5f 5d 00 00 63 96 5f 5d 02 00 10 00 00 00  _]_]..c._]......
00005020: 54 45 53 54 32 20 20 20 54 58 54 20 00 00 63 96  TEST2   TXT ..c.
00005030: 5f 5d 5f 5d 00 00 63 96 5f 5d 03 00 12 00 00 00  _]_]..c._]......
00005040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
00005050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
00005060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
00005070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
```

# cで構造を読む

```
gcc -o dump_root dump_root.c
./dump_root fat16.img
```

```
test  …/KansoOS/test/fat16   master ✘?⇡    v15.2.0    ./dump_root fat16.img
bytes/sec = 512
sec/clus  = 8
rsvd      = 8
num_fats  = 2
fat_sec   = 16
root_ent  = 512
root LBA  = 40
data LBA  = 72

# name       attr clus size
TEST1.TXT   20   38499 154975
TEST2.TXT   20   38499 220511
```
