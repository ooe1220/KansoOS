#include "../lib/stdint.h"
#include "../x86/io.h"
#include "../x86/console.h"
#include "../lib/stddef.h"

static const struct pci_dev_ent {
    uint16_t vendor;
    uint16_t device;
    const char *name;
} dev_table[] = {
    { 0x8086, 0x1237, "82441FX i440FX Host Bridge" },
    { 0x8086, 0x7000, "82371SB PIIX3 ISA Bridge"   },
    { 0x8086, 0x7010, "82371SB PIIX3 IDE"          },
    { 0x8086, 0x7113, "82371AB PIIX4 ACPI"         },
    { 0x8086, 0x100e, "82540EM Gigabit Ethernet"   },
    { 0x1234, 0x1111, "QEMU Standard VGA"          },
    { 0x8086, 0x7110, "82371AB/EB/MB PIIX4 ISA"        },
    { 0x8086, 0x7111, "82371AB/EB/MB PIIX4 IDE"        },
    { 0x8086, 0x7112, "82371AB/EB/MB PIIX4 USB"        },

    { 0x8086, 0x2448, "82801BA (ICH2) SATA/RAID"       },
    { 0x8086, 0x244a, "82801BAM (ICH2-M) IDE"         },
    { 0x8086, 0x244b, "82801BA (ICH2) IDE"            },

    { 0x8086, 0x24c2, "82801DB/DBL PRO/1000 Ethernet" },
    { 0x8086, 0x24c4, "82801DB (ICH4) IDE"            },
    { 0x8086, 0x24c5, "82801DB (ICH4) AC\'97 Audio"   },
    { 0x8086, 0x24c7, "82801DB (ICH4) SMBus"          },
    { 0x8086, 0x24ca, "82801DBM (ICH4-M) IDE"         },
    { 0x8086, 0x24cc, "82801DBM (ICH4-M) IDE"         },
    { 0x8086, 0x24cd, "82801DBM (ICH4-M) SMBus"       },

    { 0x8086, 0x3580, "6300ESB PATA/SATA IDE"         },
    { 0x8086, 0x3582, "6300ESB SATA AHCI"             },
    { 0x8086, 0x3584, "6300ESB SATA Storage"          },
    { 0x0000, 0x0000, NULL }                        /* 終端 */
};

static const struct pci_ven_ent {
    uint16_t id;
    const char *name;
} ven_table[] = {
    { 0x8086, "Intel" },
    { 0x1234, "QEMU"  },
    { 0x0000, NULL    }
};

const char *pci_vendor_name(uint16_t vid)
{
    for (int i = 0; ven_table[i].name; i++)
        if (ven_table[i].id == vid)
            return ven_table[i].name;
    return "Unknown vendor";
}

const char *pci_device_name(uint16_t vid, uint16_t did)
{
    for (int i = 0; dev_table[i].name; i++)
        if (dev_table[i].vendor == vid && dev_table[i].device == did)
            return dev_table[i].name;
    return "Unknown device";   /* 見つからない時 */
}

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

#define PCI_MAX_BAR 6

static void pci_print_bars(uint8_t bus, uint8_t dev, uint8_t fn)
{
    for (int bar = 0; bar < PCI_MAX_BAR; bar++) {
        uint8_t off = 0x10 + (bar * 4);

        uint32_t orig = pci_read32(bus, dev, fn, off);

        /* BAR 未実装（0）はスキップ */
        if (orig == 0 || orig == 0xFFFFFFFF)
            continue;

        /* サイズ判定：全ビット1を書いて読む */
        outl(PCI_CONFIG_ADDR,
             (1u << 31) |
             ((uint32_t)bus << 16) |
             ((uint32_t)dev << 11) |
             ((uint32_t)fn << 8)  |
             (off & 0xFC));
        outl(PCI_CONFIG_DATA, 0xFFFFFFFF);

        uint32_t val = pci_read32(bus, dev, fn, off);

        /* 元に戻す（重要） */
        outl(PCI_CONFIG_ADDR,
             (1u << 31) |
             ((uint32_t)bus << 16) |
             ((uint32_t)dev << 11) |
             ((uint32_t)fn << 8)  |
             (off & 0xFC));
        outl(PCI_CONFIG_DATA, orig);

        if (val == 0)
            continue;

        if (val & 0x1) {
            /* I/O BAR */
            uint32_t base = orig & ~0x3u;
            uint32_t size = ~(val & ~0x3u) + 1;
            kprintf("  BAR%d: I/O  base=%x size=%x\n", bar, base, size);
        } else {
            /* Memory BAR */
            uint32_t base = orig & ~0xFu;
            uint32_t size = ~(val & ~0xFu) + 1;
            kprintf("  BAR%d: MEM  base=%x size=%x%s\n",
                    bar, base, size,
                    (val & (1 << 3)) ? " (64bit)" : "");

            /* 64bit BAR なら次の BAR も消費する */
            if ((val & 0x6) == 0x4) {  // type=64bit
                bar++;
            }
        }
    }
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

            //kprintf("%x:%x.%d  %x:%x  class=%x.%x\n",
            //        0, dev, fn, vendor, device, cls, sub);
            kprintf("%x:%x.%d  %x:%x  %s %s  (class=%x.%x)\n",
                    0, dev, fn,
                    vendor, device,
                   pci_vendor_name(vendor),
                    pci_device_name(vendor, device),
                    cls, sub);

            /* ★BAR 表示 */
            pci_print_bars(0, dev, fn);
        }
    }
}