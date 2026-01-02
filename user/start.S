# ユーザプログラム入り口
# 簡素OSではユーザプログラムにヘッダは無く、固定でメモリ0x10000へ読み込み実行する
# 元々kernel.c → mainへ直接跳ぶ設計にしていた
# しかしmainの上に別の関数が置かれるため、逆アセンブルしてmain関数の始まるオフセットを調べ
# 0x10000 + main()オフセットの位置に直接跳んでいた。例)user_exec((void*)0x10016);
# 
# 00000000 <.data>:
#    0:	55                   	push   %ebp ← 別の関数
#    1:	89 e5                	mov    %esp,%ebp
#    3:	53                   	push   %ebx
#   ...(略)...
#   16:	55                   	push   %ebp ← mainはここから始まる
#   17:	89 e5                	mov    %esp,%ebp
#   ...(略)...
# 
# kernel.c → start.S → mainとすることで先頭に以下のような命令が入り
# main()のオフセットを調べることなく毎回固定で0x10000へ跳んで実行することが可能となる
# 00000000 <.data>:
#    0:	e8 18 00 00 00       	call   0x1d
#    5:	eb fe                	jmp    0x5
#    7:	55                   	push   %ebp
#   ...(略)... 
#   1d:	55                   	push   %ebp ← mainはここから始まる


.intel_syntax noprefix
.global _start
.extern main

_start:
    call main
1:
    ret # kernel.cのuser_exec((void*)0x10000);直後へ戻る


