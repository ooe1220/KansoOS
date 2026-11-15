#ifndef MEMMAP_H
#define MEMMAP_H

#define BOOTLOADER_ADDR     0x7C00u   /* ブートセクタ(512B)。CDブートは2048Bで0x8000を侵食 */
#define KERNEL_LOAD_ADDR    0x8000u   /* kernel.bin ロード先 (BIOS領域に迫ったら移動) */

#define USER_PROG_ADDR      0x600000u /* ユーザプログラム */
#define USER_ARG_ADDR       0x610000u /* ユーザプログラム引数 */

#define KERNEL_STACK_TOP    0x900000u /* カーネルスタック頂上*/
#define USER_STACK_TOP      0xAFF000u /* ユーザスタック頂上 */

#define USER_FB_ADDR        0xB00000u /* ユーザ用フレームバッファ */

#endif