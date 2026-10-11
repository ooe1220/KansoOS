
#include "debug/serial.h"
#include "debug/serial_print.h"
#include "lib/itoa.h"
#include "lib/stdint.h"


void serial_print(const char* format, ...) {
    char* arg_ptr = (char*)&format + sizeof(char*);
    const char* p = format;
    char buffer[32];
    
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case '%':
                    serial_putc('%');
                    break;
                    
                case 'c':
                    serial_putc(*((char*)arg_ptr));
                    arg_ptr += sizeof(char);
                    break;
                    
                case 's':
                    serial_puts(*((char**)arg_ptr));
                    arg_ptr += sizeof(char*);
                    break;
                    
                case 'd':
                case 'i':
                    itoa(*((int*)arg_ptr), buffer);
                    serial_puts(buffer);
                    arg_ptr += sizeof(int);
                    break;
                    
                case 'u':
                    utoa(*((unsigned int*)arg_ptr), buffer, 10);
                    serial_puts(buffer);
                    arg_ptr += sizeof(unsigned int);
                    break;
                    
                case 'x':
                    utoa(*((unsigned int*)arg_ptr), buffer, 16);
                    serial_puts(buffer);
                    arg_ptr += sizeof(unsigned int);
                    break;
                    
                default:
                    serial_putc('%');
                    serial_putc(*p);
                    break;
            }
        } else {
            serial_putc(*p);
        }
        p++;
    }
}

//ESPの確認　関数化不可、デバッグ箇所へ貼る
/*
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    serial_print("ESP = 0x%x\n", esp);
*/

// esp自体はこの関数が変更するが、何か関数呼ぶ前後で相対的に一致するかは確認できる
// -O2してはいけない。
void debug_dump_regs(void) {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eip, eflags;
    uint16_t cs, ds, es, fs, gs;
    
    asm volatile ("mov %%eax, %0" : "=r"(eax));
    asm volatile ("mov %%ebx, %0" : "=r"(ebx));
    asm volatile ("mov %%ecx, %0" : "=r"(ecx));
    asm volatile ("mov %%edx, %0" : "=r"(edx));
    asm volatile ("mov %%esi, %0" : "=r"(esi));
    asm volatile ("mov %%edi, %0" : "=r"(edi));
    asm volatile ("mov %%ebp, %0" : "=r"(ebp));
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    // eip = [ebp + 4]（call がスタックに積んだ戻りアドレス）
    eip = *(uint32_t*)(ebp + 4);
    asm volatile ("pushf; pop %0" : "=r"(eflags));
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    asm volatile ("mov %%ds, %0" : "=r"(ds));
    asm volatile ("mov %%es, %0" : "=r"(es));
    asm volatile ("mov %%fs, %0" : "=r"(fs));
    asm volatile ("mov %%gs, %0" : "=r"(gs));
    
    serial_print("EAX=0x%x EBX=0x%x ECX=0x%x EDX=0x%x\n", eax, ebx, ecx, edx);
    serial_print("ESI=0x%x EDI=0x%x EBP=0x%x ESP=0x%x\n", esi, edi, ebp, esp);
    serial_print("EIP=0x%x EFLAGS=0x%x\n", eip, eflags);
    serial_print("CS=0x%x DS=0x%x ES=0x%x FS=0x%x GS=0x%x\n", cs, ds, es, fs, gs);
}
//objdump -d -M intel build/serial_print.o

void debug_enter(const char *func) {
    serial_print(">> %s\n", func);
}

void debug_leave(const char *func) {
    serial_print("<< %s\n", func);
}

void debug_dump_mem(uint32_t addr, int bytes) {
    int i;
    for (i = 0; i < bytes; i += 16) {
        serial_print("[0x%x] ", addr + i);
        int j;
        for (j = 0; j < 16 && (i + j) < bytes; j++) {
            serial_print("0x%x ", *(uint8_t*)(addr + i + j));
        }
        serial_putc('\n');
    }
}

// 呼び方
// uint32_t esp;
// asm volatile ("mov %%esp, %0" : "=r"(esp));
// debug_dump_stack(esp, 32, 32);
void debug_dump_stack(uint32_t esp, int below, int above) {
    serial_print("--- stack dump (esp=0x%x) ---\n", esp);
    
    // 下側（高アドレス側、これより前に積まれてたもの）
    if (above > 0) {
        serial_print("above esp:\n");
        uint32_t addr = esp + 4;
        int i;
        for (i = 0; i < above && i < 64; i += 4) {
            serial_print("  [0x%x] = 0x%x\n", addr + i, *(uint32_t*)(addr + i));
        }
    }
    
    // 上側（低アドレス側、これから使われる領域）
    if (below > 0) {
        serial_print("below esp:\n");
        int i;
        for (i = 0; i < below && i < 64; i += 4) {
            uint32_t addr = esp - 4 - i;
            serial_print("  [0x%x] = 0x%x\n", addr, *(uint32_t*)addr);
        }
    }
    
    serial_print("--- end stack dump ---\n");
}

