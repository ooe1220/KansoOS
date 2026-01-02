ENTRY(start)

SECTIONS {
    /* カーネルはメモリ0x10000〜の領域に読み込む */
    . = 0x8000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
