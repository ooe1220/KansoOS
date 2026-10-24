# 序
32bit x86で動作するOS(簡素OS)を開発しています。
実用性は求めておらず、原理を理解する為にソースコードを通読できる規模のOSを目指しています。
最終的にはGUIを目指していますが現在は32bitにしただけのDOS状態です。
システムコールは`int 80h`経由で実装しておりますが、権限やスタック等はまだ分けられていません。

# 動作
`./build.sh`を実行するとビルドから起動まで一気にできます。
何故Makefileを使わないのか疑問に思われるかも知れませんが、どうしても苦手で保守が大変な為、使用はしていません。

# 環境

## 開発環境

```
OS : Ubuntu 22.04.5 LTS
CPU : Intel(R) Core(TM) i5-7200U CPU @ 2.50GHz
コンパイラ : gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04.3) 
アセンブラ : NASM version 2.15.05
```

## 動作環境

```
仮想環境 : QEMU emulator version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.31)
実機検証 : 東芝 dynabook SS M10 (Intel(R) Pentium(R) M Processor 1.10GHz)
```

# 起動の流れ

```
[ 簡易BIOS ] ─int19h→ [ VBR@0x0600 ] → [ MBR@0x7C00 ] → [ kernel.bin@0x8000 ]
                                                              │
                                                     switch32.asm
                                                              │
                                                     kernel_main (32bit)
```

## 簡易BIOS

※ QEMU向けに開発している簡素OSを起動する為だけの簡易BIOSです。
　実機のBIOSはもっと複雑です。実機で動かす際は実機のBIOSを使用します。
　

1. CPUの故障診断
2. キーボードの初期設定
3. 割り込みIVTの登録
4. VGAをテキストモード80*25へ設定
5. A20設定
6. 記憶媒体からLBA0のブートローダをメモリ上0x7C00へ展開してjmp 0x0000:0x7C00

## 簡素OS

VBR : メモリ上0x0600へ自己退避し、MBRを0x7C00へ展開してjmp 0x0000:0x7C00
MBR : kernel.binを64kb分メモリ0x8000へ展開してjmp 0x0000:0x8000
switch32.asm(kernel.bin先頭) : A20を設定、VBE800*600*16bpp設定、32ビットモード切り替え、call kernel_main

# 簡易BIOSに関して

簡素BIOSを起動するだけのかなり割り切った実装となっています。

## int10h

### AH=00H
画面に1文字表示

### AH=0EH 
固定でVGAをテキストモード3(80*25)を設定

### AX=4F00
VBEに対応しているか否かを返す。(対応)

### AX=4F01 
VRAM開始アドレス、幅、高さ等の表示に必要な構造体を返す

### AX=4F02
VBE800*600*16bpp設定

## int12h

1MB 以下の通常メモリが何 KBあるかをAXレジスタに返す

## int13h

※AH=0x02も作っていましたが、CHSの計算はややこしいので消しました。

### AH=0x42

現在はATAのみ対応しています。
ATAPI(光学ドライバ実装中)


## int19h

`int13h`を呼び出し、ブートローダを0x7C00へ展開してjmp 0x0000:0x7C00


# 開発予定

## 簡易BIOS
- BIOSのメモリ診断
- int 13hにCD/DVD追加

## 簡素OS
- ATAPI(光学)ドライバ
- CD/DVD向けのファイルシステム
- ringの切り替え
- 複数プロセス



