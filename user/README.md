
# 簡素OSのsyscall流れ

## ユーザ側

`lib/os.h` EAX=syscall番号を指定して、`int 0x80`で呼ぶ。

## OS側

`x86/syscall.c`内で定義した`syscall_handler`関数だけ`syscall_entry.S`内で実装している。

ユーザ側から`int 0x80`が実行されると、`syscall_entry.S`内の`syscall_handler`へ飛ぶ。

`syscall_handler`内でEAXを元にどのシステムコールかを判定し、EBX以降のレジスタから引数を取り出してスタックへ積んだあと、`x86/syscall.c`内の関数を呼び出し、その中からカーネル関数を呼び出す。

