#include "command.h"
#include "arch/x86/console.h"
#include "arch/x86/io.h"
#include "lib/string.h"
#include "lib/string.h"
#include "lib/stdint.h"

void execute_command(const char *line) {
    if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help   - Show this message\n");
        kputs("  clear  - Clear screen\n");
        kputs("  reboot - Reboot CPU\n");
    } else if (strcmp(line, "clear") == 0) {
        for (uint16_t *vga = (uint16_t*)0xB8000; vga < (uint16_t*)(0xB8000 + 80*25*2); vga++)
            *vga = 0x0700;
    } else if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
    } else if (line[0] != 0) {
        kputs("\nUnknown command: ");
        kputs(line);
    }
}

