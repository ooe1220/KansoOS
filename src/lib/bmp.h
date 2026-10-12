#ifndef BMP_H
#define BMP_H

void draw_bmp(int dest_x, int dest_y, const uint8_t *bmp_data);

void draw_bmp_scaled(int dest_x, int dest_y, const uint8_t *bmp_data, int scale);

void show_boot_logo();

void show_penguin(void);

#endif
