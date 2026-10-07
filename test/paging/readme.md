
ページングの実験をする。
ページングのみに焦点を当てる為、GRUBを使い、32bitモード移行の過程は省略する。

# 実行命令

```
mkdir tmp

nasm -f elf32 start.asm -o tmp/start.o
gcc -m32 -ffreestanding -nostdlib -c kernel.c -o tmp/kernel.o
ld -m elf_i386 -T link.ld -o tmp/kernel.bin tmp/start.o tmp/kernel.o

grub-file --is-x86-multiboot tmp/kernel.bin

mkdir -p iso/boot/grub
cp tmp/kernel.bin iso/boot/kernel.bin
cp grub.cfg iso/boot/grub/grub.cfg
grub-mkrescue -o tmp/myos.iso iso

qemu-system-i386 -cdrom tmp/myos.iso
```

# 実アドレス→CPU変換→仮想アドレス


仮想アドレス(32bit)
 = [上位10bit][中10bit][下位12bit]
 
 ビット範囲	使われ方
31〜22 (10bit)	page_dir のインデックス
21〜12 (10bit)	page_table のインデックス
11〜0 (12bit)	ページ内オフセット

下位12bit = 4096 = 4KB。1ページは4KB。

page_dir[1024]←1CPUにつき1つだけ
  1024個のエントリを持つ
    各エントリが「どのページテーブルを使うか」を指す
  CR3レジスタがこの配列の物理アドレスを指す
    インデックス0〜1023 が仮想アドレスの 0〜4GB をカバー（1エントリ = 4MB


page_dir[0]   = (uint32_t)page_table_0 | 3;
page_dir[768] = (uint32_t)page_table_c0 | 3;





流れ

プログラムが0xC00B8000参照する

1.CR3(page_dirの物理アドレス)を見る

2.page_dir からページテーブルを探す
page_dir[768] → page_table_c0 の物理アドレスが出てくる

3.page_table から物理ページを探す
page_table_c0[0] を読む→ 物理アドレス 0x00000000 が出てくる

4.オフセットを足す
0x00000000 + 0xB8000 = 0x000B8000

5.結果
仮想アドレス 0xC00B8000 → 物理アドレス 0x000B8000



0xC00B8000
=1100000000 | 0000000000 | 1011100000000000　(10ビット・10ビット・12ビットで区切る)

区切り	2進数	10進数	16進数
page_dir のインデックス	1100000000	768​	0x300
page_table_c0 のインデックス	0000000000	0​	0x000
ページ内の位置	1011100000000000	—	0xB8000​

部分	数字	CPUはこれを何に使うか
左10ビット	768	page_dir の768番目を見る
真ん中10ビット	0	page_table の0番目を見る
右12ビット	0xB8000	そのページの中の 0xB8000 バイト目


1. CR3 → page_dir の先頭アドレス
2. page_dir[768] を読む → page_table_c0 のアドレス
3. page_table_c0[0] を読む → 物理アドレス


