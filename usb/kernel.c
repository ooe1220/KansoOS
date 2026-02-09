#include "x86/io.h"
#include "x86/console.h"
#include "drivers/cmos.h"
#include "x86/pic.h"
#include "x86/idt.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "x86/panic.h"
#include "x86/syscall.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "../src/kernel/command.h"
#include "../src/kernel/user_exec.h"
#include "mem/memory.h"

void init();
uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pci_scan();

// PCI Configuration Address / Data ポート
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

void kernel_main() {

    kputs("--------------------------------------------\n");
    kputs("       DEVELOPMENT KERNEL BOOTED           \n");
    kputs("        (Experimental / Debug Build)       \n");
    kputs("--------------------------------------------\n");
        
    init();
        
    pci_scan();
    
    for(;;){
        asm volatile("hlt");
    }

}

void init(){
    idt_init(); // IDT初期化
    pic_init(); // PIC初期化
    pic_unmask_irq(1); // PICのIRQ1(キーボード割り込み)を有効化
    keyboard_init(); //  IDT(0x21=33)へIRQ1（キーボード）処理を登録
    exception_init(); // IDT(0〜31=0x00〜0x1F)へCPU例外処理を登録(panic)
    init_syscall(); // IDT 0x80へシステムコールを登録　Linuxの様にint0x80経由でシステムコールを呼び出す
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
}

// PCIから32bit読み込み
uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (1U << 31)
                     | ((uint32_t)bus << 16)
                     | ((uint32_t)device << 11)
                     | ((uint32_t)func << 8)
                     | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

// PCIスキャン
void pci_scan() {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint8_t func = 0; // func0だけチェック
            uint32_t val = pci_read32(bus, device, func, 0x00);
            
            if(val!=-1){
                kprintf("%d\n", val);
            }
            
            uint16_t vendor_id = val & 0xFFFF;
            uint16_t device_id = (val >> 16) & 0xFFFF;

            if (val == 0xFFFFFFFF) continue;  // 存在しないデバイスはスキップ

            //kputs("PCI ");

            // Header Type の bit7 = マルチファンクション
            /*
            uint8_t header_type = (pci_read32(bus, device, func, 0x0C) >> 16) & 0xFF;
            if (header_type & 0x80) { // bit7が1ならマルチファンクション
                for (func = 1; func < 8; func++) {
                    val = pci_read32(bus, device, func, 0x00);
                    vendor_id = val & 0xFFFF;
                    if (vendor_id != 0xFFFF) {
                        //kputs("  Func "); 
                    }
                }
            }*/
        }
    }
}

