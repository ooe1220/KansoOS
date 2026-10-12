#include "./kernel/penguin_bmp.h"
#include "./x86/vbe.h"

// BMPを (dest_x, dest_y) に描画する
void draw_bmp(int dest_x, int dest_y, const uint8_t *bmp_data) {
    if (!bmp_data || !vbe) return;

    uint8_t *fb_base = (uint8_t *)(uintptr_t)vbe->vram_addr;
    uint32_t fb_pitch = vbe->lfb_pitch;
    uint32_t fb_width = vbe->x_res;
    uint32_t fb_height = vbe->y_res;

    // BMPヘッダー読み取り（リトルエンディアン）
    uint16_t bf_type = *(uint16_t *)(bmp_data + 0x00);
    if (bf_type != 0x4D42) return; // 'BM' でなければ終了

    uint32_t bf_offbits = *(uint32_t *)(bmp_data + 0x0A);
    int32_t  bi_width   = *(int32_t  *)(bmp_data + 0x12);
    int32_t  bi_height  = *(int32_t  *)(bmp_data + 0x16);
    uint16_t bi_bitcount = *(uint16_t *)(bmp_data + 0x1C);

    if (bi_bitcount != 24) return; // 24BPP以外は未対応

    int is_topdown = (bi_height < 0);
    int height = is_topdown ? -bi_height : bi_height;
    int width = bi_width;

    // 各行のバイト数（パディング込み）
    int row_bytes = ((width * 3 + 3) / 4) * 4;

    const uint8_t *pixel_data = bmp_data + bf_offbits;

    for (int y = 0; y < height; y++) {
        // BMPはボトムアップの場合、下の行から格納されている
        int bmp_row = is_topdown ? y : (height - 1 - y);
        const uint8_t *src_row = pixel_data + bmp_row * row_bytes;

        for (int x = 0; x < width; x++) {
            int fb_x = dest_x + x;
            int fb_y = dest_y + y;

            if (fb_x < 0 || fb_x >= (int)fb_width || fb_y < 0 || fb_y >= (int)fb_height)
                continue;

            uint8_t *fb = fb_base + fb_y * fb_pitch + fb_x * 3;
            const uint8_t *px = src_row + x * 3;

            // BMP: BGR → VBE: BGR（そのままコピー）
            fb[0] = px[0];  // B
            fb[1] = px[1];  // G
            fb[2] = px[2];  // R
        }
    }
}

void show_penguin(void) {
    // 画面中央に表示
    //int x = (fb_width - 80) / 2;
    //int y = (fb_height - 80) / 2;
    int x = 0;
    int y = 0;
    draw_bmp(x, y, src_kernel_penguin_bmp);
}
