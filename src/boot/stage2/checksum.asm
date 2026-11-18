bits 32

checksum:

    mov   edi, 0x200000
    mov   ecx, 65536
    call  calc

    ; (qemu) x/4xb 0x8006
    ; (qemu) info registers
    cmp   eax, [expected_sum]     ; 足し算だけで判定
    jne   .fail
    ret

.fail:
    cli
    hlt
    jmp   .fail

;   edi = 先頭, ecx = バイト数
;   戻り: eax = 加算値
calc:
    xor   eax, eax
.loop:
    movzx ebx, byte [edi]
    add   eax, ebx
    inc   edi
    loop  .loop
    ret
