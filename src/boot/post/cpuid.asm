bits 16

; ============================================================
; CPUID 対応判定（実モード）
;   EFLAGS の bit21 (ID) を反転できるかを見る
;   戻り値: AL = 1 : CPUID あり / 0 : なし
; ============================================================
; 注意　CPUIDの無い環境が無く、無い場合は未検証。今後86BOXで確認予定
cpuid_check:
    pushfd                  ; 現在の EFLAGS を退避
    pop     eax
    mov     ecx, eax        ; 比較用に保存
    xor     eax, 0x200000   ; bit21 (ID) を反転
    push    eax
    popfd
    pushfd
    pop     eax             ; 反転が効いたか読み戻す
    push    ecx
    popfd                   ; EFLAGS を復元

    xor     eax, ecx        ; bit21 が変化していれば 0 以外
    jz      .no_cpuid

    mov si, msg_cpuid_ok
    call print_string

    mov     al, 1
    jmp     .done

.no_cpuid:

    mov si, msg_no_cpuid
    call print_string

    xor     al, al
.done:
    ret

msg_cpuid_ok    db "CPUID supported", 0x0D, 0x0A, 0
msg_no_cpuid    db "CPUID not supported", 0x0D, 0x0A, 0