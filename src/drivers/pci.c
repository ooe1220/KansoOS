#include "../lib/stdint.h"
#include "../x86/io.h"
#include "../x86/console.h"

#define PCI_CONFIG_ADDR  0xCF8
#define PCI_CONFIG_DATA  0xCFC

static uint32_t pci_read32(uint8_t bus, uint8_t dev,
                           uint8_t fn, uint8_t off)
{
    uint32_t addr = (1u << 31) |
                    ((uint32_t)bus  << 16) |
                    ((uint32_t)dev  << 11) |
                    ((uint32_t)fn   << 8)  |
                    (off & 0xFC);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}


//重くなるのでバス0のみ
void pci_scan(void)
{
    for (uint8_t dev = 0; dev < 32; dev++) {
        for (uint8_t fn = 0; fn < 8; fn++) {
            uint32_t id = pci_read32(0, dev, fn, 0x00);
            uint16_t vendor = id & 0xFFFF;
            uint16_t device = id >> 16;

            if (vendor == 0xFFFF) {
                // デバイスが無い。fn==0ならdev自体が空
                if (fn == 0) break;
                continue;
            }

            uint32_t cc = pci_read32(0, dev, fn, 0x08);
            uint8_t cls = cc >> 24;
            uint8_t sub = (cc >> 16) & 0xFF;

            kprintf("%x:%x.%d  %x:%x  class=%x.%x\n",
                    0, dev, fn, vendor, device, cls, sub);
        }
    }
}