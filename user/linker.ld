ENTRY(_start)

SECTIONS
{
    /* ユーザ関数はメモリ0x10000〜の領域に読み込む */
    . = 0x10000;

    /* 実行コード */
    .text : {
        *(.text*)
    }

    /* 読み取り専用データ（文字列など） */
    .rodata : {
        *(.rodata*)
    }

    /* 初期化済みデータ */
    .data : {
        *(.data*)
    }

    /* 未初期化データ（bin には出さない） */
    .bss : {
        *(.bss*)
        *(COMMON)
    }
}

