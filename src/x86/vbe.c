#include "vbe.h"
#include "x86/console.h"
#include "lib/stddef.h"

vbe_mode_info_t *vbe = NULL;

void vbe_show_info(void) {
    uint32_t addr = MODEINFO_PTR;
    if (addr == 0) {
        kputs("VBE: no info available\n");
        return;
    }

    vbe = (vbe_mode_info_t *)addr;

    if ((vbe->mode_attr & 1) == 0) {
        kputs("VBE: mode not supported\n");
        vbe = NULL;
        return;
    }

    kprintf("VBE: %dx%d %dbpp VRAM=0x%x\n",
        vbe->x_res, vbe->y_res, vbe->bpp, vbe->vram_addr);
}
