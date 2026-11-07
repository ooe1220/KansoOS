#include "lib/gfx.h"

// 関数が増えたら例外６起こる
int main(void)
{
    /* 背景を黒でクリア */
    g_rect(0, 0, FB_WIDTH, FB_HEIGHT, COLOR_BLACK);

    /* 枠：緑の矩形 */
    g_rect(20, 20, FB_WIDTH - 40, FB_HEIGHT - 40, COLOR_GREEN);

    /* 対角線：赤 */
    g_line(20, 20, FB_WIDTH - 20, FB_HEIGHT - 20, COLOR_RED);

    /* 逆対角線：青 */
    g_line(FB_WIDTH - 20, 20, 20, FB_HEIGHT - 20, COLOR_BLUE);

    /* 十字：白 */
    g_line(FB_WIDTH / 2, 0, FB_WIDTH / 2, FB_HEIGHT, COLOR_WHITE);

    /* 1ドットだけ確認用 */
    g_pixel(FB_WIDTH / 2, FB_HEIGHT / 2, COLOR_WHITE);

    g_flip();  // 描画

    // 遅延
    for (volatile uint32_t i = 0; i < 400000000; i++)
        asm volatile("nop");

    // 退出前に背景色で画面クリア
    g_rect(0, 0, FB_WIDTH, FB_HEIGHT, BG_COLOR);
    g_flip(); 

    return 0;
    //for (;;) asm volatile("hlt");
}