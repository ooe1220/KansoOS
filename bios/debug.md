BIOS開発ではprintf()やint10hが使用出来ない為、以下の方法でデバッグを行います。

# -serial stdio

## 到達確認
```
   mov dx, 0x3F8
   mov al, '-'
   out dx, al
```

# -monitor stdio

## レジスタ値確認可能
`hlt`で止めて、`(qemu)info registers`実行

## レジスタ退避方法

```
saved_ax dw 0

mov [saved_ax], ax
mov dx,[saved_dx]
```
